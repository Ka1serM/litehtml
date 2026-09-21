#include "html.h"
#include "el_text.h"
#include "render_item.h"
#include "document_container.h"
#include <string_view>

namespace {

std::size_t previous_utf8_codepoint(const std::string& text, const std::size_t end) {
    if (end == 0) return 0;
    std::size_t start = end - 1;
    while (start > 0 && (static_cast<unsigned char>(text[start]) & 0xc0u) == 0x80u) --start;
    return start;
}

std::string ellipsize(const std::string_view text, const litehtml::pixel_t max_width,
                     const litehtml::uint_ptr font, litehtml::document_container* container) {
    constexpr std::string_view ellipsis = "\xE2\x80\xA6";
    if (container->text_width(ellipsis.data(), font) > max_width) return {};
    if (container->text_width(text.data(), font) <= max_width) return std::string(text);

    std::string result(text);
    while (!result.empty()) {
        result.resize(previous_utf8_codepoint(result, result.size()));
        std::string candidate = result;
        candidate += ellipsis;
        if (container->text_width(candidate.c_str(), font) <= max_width) return candidate;
    }
    return std::string(ellipsis);
}

}  // namespace

litehtml::el_text::el_text(const char* text, const document::ptr& doc) :
    element(doc)
{
    if(text)
    {
        m_text = text;
    }
    m_use_transformed = false;
    m_draw_spaces     = true;
    css_w().set_display(display_inline_text);
}

void litehtml::el_text::get_content_size(size& sz, pixel_t /*max_width*/)
{
    sz = m_size;
}

void litehtml::el_text::get_text(std::string& text) const
{
    text += m_text;
}

void litehtml::el_text::set_data(const char* data)
{
    m_text = data ? data : "";
    m_transformed_text.clear();
    m_use_transformed = false;
    compute_styles(false);
    // Text nodes can be updated after the persistent render tree has been
    // laid out (range outputs are a common example). Their width and content
    // changed, so invalidate the containing layout instead of relying on a
    // later resize to change the root cache key.
    if(const auto render = get_render_item()) render->invalidate_layout();
}

void litehtml::el_text::compute_styles(bool /*recursive*/)
{
    element::ptr el_parent = parent();
    if(el_parent)
    {
        css_w().line_height_w() = el_parent->css().line_height();
        css_w().set_font(el_parent->css().get_font());
        css_w().set_font_metrics(el_parent->css().get_font_metrics());
        css_w().set_white_space(el_parent->css().get_white_space());
        css_w().set_text_transform(el_parent->css().get_text_transform());
    }
    css_w().set_display(display_inline_text);
    css_w().set_float(float_none);

    if(m_css.get_text_transform() != text_transform_none)
    {
        m_transformed_text = m_text;
        m_use_transformed  = true;
        get_document()->container()->transform_text(m_transformed_text, m_css.get_text_transform());
    } else
    {
        m_use_transformed = false;
    }

    element::ptr p = parent();
    while(p && p->css().get_display() == display_inline)
    {
        if(p->css().get_position() == element_position_relative)
        {
            css_w().set_offsets(p->css().get_offsets());
            css_w().set_position(element_position_relative);
            break;
        }
        p = p->parent();
    }
    if(p)
    {
        css_w().set_position(element_position_static);
    }

    if(is_white_space())
    {
        m_transformed_text = " ";
        m_use_transformed  = true;
    } else
    {
        if(m_text == "\t")
        {
            m_transformed_text = "    ";
            m_use_transformed  = true;
        }
        if(m_text == "\n" || m_text == "\r")
        {
            m_transformed_text = "";
            m_use_transformed  = true;
        }
    }

    font_metrics fm;
    uint_ptr     font = 0;
    if(el_parent)
    {
        font = el_parent->css().get_font();
        fm   = el_parent->css().get_font_metrics();
    }
    if(is_break() || !font)
    {
        m_size.height = 0;
        m_size.width  = 0;
    } else
    {
        m_size.height = fm.height;
        m_size.width  = get_document()->container()->text_width(
            m_use_transformed ? m_transformed_text.c_str() : m_text.c_str(), font);
    }
    m_draw_spaces = fm.draw_spaces;
}

void litehtml::el_text::draw(uint_ptr hdc, pixel_t x, pixel_t y, const position* clip,
                             const std::shared_ptr<render_item>& ri)
{
    if(is_white_space() && !m_draw_spaces)
    {
        return;
    }

    position pos  = ri->pos();
    pos.x        += x;
    pos.y        += y;
    pos.round();

    if(pos.does_intersect(clip))
    {
        element::ptr el_parent = parent();
        if(el_parent)
        {
            document::ptr doc = get_document();

            uint_ptr font = el_parent->css().get_font();
            if(font)
            {
                web_color color = el_parent->css().get_color();
                const char* source_text = m_use_transformed ? m_transformed_text.c_str() : m_text.c_str();
                std::string shortened_text;
                auto overflow_element = el_parent;
                while(overflow_element && overflow_element->css().get_text_overflow() != text_overflow_ellipsis)
                {
                    overflow_element = overflow_element->parent();
                }

                const auto overflow_render = overflow_element ? overflow_element->get_render_item() : nullptr;
                pixel_t text_offset_x = ri->pos().x;
                auto containing_render = ri->parent();
                while(containing_render && containing_render != overflow_render)
                {
                    text_offset_x += containing_render->pos().x;
                    containing_render = containing_render->parent();
                }

                if(overflow_render && containing_render == overflow_render &&
                   overflow_element->css().get_white_space() == white_space_nowrap &&
                   overflow_element->css().get_overflow() != overflow_visible)
                {
                    pixel_t available_width = overflow_render->pos().width - text_offset_x;
                    if(available_width <= 0_px)
                    {
                        available_width = overflow_render->width() - overflow_render->content_offset_width() - text_offset_x;
                    }
                    if(available_width > 0_px)
                    {
                        shortened_text = ellipsize(source_text, available_width, font, doc->container());
                        source_text = shortened_text.c_str();
                    }
                }
                doc->container()->draw_text(hdc, source_text, font, color, pos);
            }
        }
    }
}

std::string litehtml::el_text::dump_get_name()
{
    return "text: \"" + get_escaped_string(m_text) + "\"";
}

std::vector<std::tuple<std::string, std::string>> litehtml::el_text::dump_get_attrs()
{
    return {};
}

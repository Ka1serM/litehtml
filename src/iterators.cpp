#include "iterators.h"
#include "render_item.h"

litehtml::elements_iterator::elements_iterator(bool return_parents, iterator_selector* go_inside,
                                               iterator_selector* select) :
    m_go_inside(go_inside),
    m_select(select),
    m_return_parent(return_parents)
{
}

bool litehtml::elements_iterator::go_inside(const std::shared_ptr<render_item>& el)
{
    return /*!el->children().empty() &&*/ m_go_inside && m_go_inside->select(el);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool litehtml::go_inside_inline::select(const std::shared_ptr<render_item>& el)
{
    return el->src_el()->css().get_display() == display_inline && el->src_el()->css().get_float() == float_none;
}

bool litehtml::inline_selector::select(const std::shared_ptr<render_item>& el)
{
    return el->src_el()->css().get_display() == display_inline_text ||
           el->src_el()->css().get_display() == display_inline_table ||
           el->src_el()->css().get_display() == display_inline_block ||
           el->src_el()->css().get_display() == display_inline_flex || el->src_el()->css().get_float() != float_none;
}

bool litehtml::go_inside_table::select(const std::shared_ptr<render_item>& el)
{
    return el->src_el()->css().get_display() == display_table_row_group ||
           el->src_el()->css().get_display() == display_table_header_group ||
           el->src_el()->css().get_display() == display_table_footer_group;
}

bool litehtml::table_rows_selector::select(const std::shared_ptr<render_item>& el)
{
    return el->src_el()->css().get_display() == display_table_row;
}

bool litehtml::table_cells_selector::select(const std::shared_ptr<render_item>& el)
{
    return el->src_el()->css().get_display() == display_table_cell;
}

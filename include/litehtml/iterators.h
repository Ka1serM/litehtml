#ifndef LITEHTML_ITERATORS_H
#define LITEHTML_ITERATORS_H

#include "types.h"
#include "render_item.h"

namespace litehtml
{
    class render_item;

    class iterator_selector
    {
      public:
        virtual bool select(const std::shared_ptr<render_item>& el) = 0;

      protected:
        ~iterator_selector() = default;
    };

    enum iterator_item_type
    {
        iterator_item_type_child,
        iterator_item_type_start_parent,
        iterator_item_type_end_parent
    };

    class elements_iterator
    {
      private:
        iterator_selector* m_go_inside;
        iterator_selector* m_select;
        bool               m_return_parent;

        /**
         * Checks if iterator should go inside the element
         *
         * @param el element to check
         * @return true to go inside
         */
        bool go_inside(const std::shared_ptr<render_item>& el);

      public:
        elements_iterator(bool return_parents, iterator_selector* go_inside, iterator_selector* select);
        ~elements_iterator() = default;

        template<typename Func>
        void process(const std::shared_ptr<render_item>& container, Func&& func)
        {
            process_impl(container, func);
        }

      private:
        void next_idx();

        template<typename Func>
        void process_impl(const std::shared_ptr<render_item>& container, Func& func)
        {
            for(auto& el : container->children())
            {
                if(go_inside(el))
                {
                    if(m_return_parent)
                    {
                        func(el, iterator_item_type_start_parent);
                    }
                    process_impl(el, func);
                    if(m_return_parent)
                    {
                        func(el, iterator_item_type_end_parent);
                    }
                } else if(!m_select || m_select->select(el))
                {
                    func(el, iterator_item_type_child);
                }
            }
        }
    };

    class go_inside_inline final : public iterator_selector
    {
      public:
        bool select(const std::shared_ptr<render_item>& el) override;
    };

    class inline_selector final : public iterator_selector
    {
      public:
        bool select(const std::shared_ptr<render_item>& el) override;
    };

    class go_inside_table final : public iterator_selector
    {
      public:
        bool select(const std::shared_ptr<render_item>& el) override;
    };

    class table_rows_selector final : public iterator_selector
    {
      public:
        bool select(const std::shared_ptr<render_item>& el) override;
    };

    class table_cells_selector final : public iterator_selector
    {
      public:
        bool select(const std::shared_ptr<render_item>& el) override;
    };
} // namespace litehtml

#endif // LITEHTML_ITERATORS_H

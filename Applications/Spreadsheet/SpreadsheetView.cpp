/*
 * Copyright (c) 2020, the SerenityOS developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "SpreadsheetView.h"
#include "SpreadsheetModel.h"
#include <LibGUI/BoxLayout.h>
#include <LibGUI/HeaderView.h>
#include <LibGUI/ModelEditingDelegate.h>
#include <LibGUI/Painter.h>
#include <LibGUI/TableView.h>
#include <LibGfx/Palette.h>

namespace Spreadsheet {

SpreadsheetView::~SpreadsheetView()
{
}

void SpreadsheetView::EditingDelegate::set_value(const GUI::Variant& value)
{
    if (m_has_set_initial_value)
        return StringModelEditingDelegate::set_value(value);

    m_has_set_initial_value = true;
    const auto option = m_sheet.at({ m_sheet.column(index().column()), (size_t)index().row() });
    if (option)
        return StringModelEditingDelegate::set_value(option->source());

    StringModelEditingDelegate::set_value("");
}

SpreadsheetView::SpreadsheetView(Sheet& sheet)
    : m_sheet(sheet)
{
    set_layout<GUI::VerticalBoxLayout>().set_margins({ 2, 2, 2, 2 });
    m_table_view = add<GUI::TableView>();
    m_table_view->row_header().set_visible(true);
    m_table_view->set_model(SheetModel::create(*m_sheet));

    m_table_view->set_row_height(18);

    set_focus_proxy(m_table_view);

    // FIXME: This is dumb.
    for (size_t i = 0; i < m_sheet->column_count(); ++i) {
        m_table_view->set_column_painting_delegate(i, make<TableCellPainter>(*m_table_view));
        m_table_view->set_column_width(i, 50);
        m_table_view->set_column_header_alignment(i, Gfx::TextAlignment::Center);
    }

    m_table_view->set_alternating_row_colors(false);
    m_table_view->set_highlight_selected_rows(false);
    m_table_view->set_editable(true);
    m_table_view->aid_create_editing_delegate = [&](auto&) {
        return make<EditingDelegate>(*m_sheet);
    };

    m_table_view->on_selection_change = [&] {
        m_sheet->selected_cells().clear();
        for (auto& index : m_table_view->selection().indexes()) {
            Position position { m_sheet->column(index.column()), (size_t)index.row() };
            m_sheet->selected_cells().set(position);
        }

        if (m_table_view->selection().is_empty() && on_selection_dropped)
            return on_selection_dropped();

        Vector<Position> selected_positions;
        selected_positions.ensure_capacity(m_table_view->selection().size());
        for (auto& selection : m_table_view->selection().indexes())
            selected_positions.empend(m_sheet->column(selection.column()), (size_t)selection.row());

        if (on_selection_changed) {
            on_selection_changed(move(selected_positions));
            m_table_view->model()->update();
            m_table_view->update();
        };
    };
}

void SpreadsheetView::hide_event(GUI::HideEvent&)
{
    if (on_selection_dropped)
        on_selection_dropped();
}

void SpreadsheetView::show_event(GUI::ShowEvent&)
{
    if (on_selection_changed && !m_table_view->selection().is_empty()) {
        Vector<Position> selected_positions;
        selected_positions.ensure_capacity(m_table_view->selection().size());
        for (auto& selection : m_table_view->selection().indexes())
            selected_positions.empend(m_sheet->column(selection.column()), (size_t)selection.row());

        on_selection_changed(move(selected_positions));
    }
}

void SpreadsheetView::TableCellPainter::paint(GUI::Painter& painter, const Gfx::IntRect& rect, const Gfx::Palette& palette, const GUI::ModelIndex& index)
{
    // Draw a border.
    // Undo the horizontal padding done by the table view...
    auto cell_rect = rect.inflated(m_table_view.horizontal_padding() * 2, 0);

    painter.draw_line(cell_rect.bottom_left(), cell_rect.bottom_right(), palette.ruler());
    painter.draw_line(cell_rect.top_right(), cell_rect.bottom_right(), palette.ruler());

    if (m_table_view.selection().contains(index)) {
        Color fill_color = palette.selection();
        fill_color.set_alpha(80);
        painter.fill_rect(cell_rect, fill_color);
    }
    if (m_table_view.cursor_index() == index)
        painter.draw_rect(cell_rect, palette.text_cursor());

    auto text_color = index.data(GUI::ModelRole::ForegroundColor).to_color(palette.color(m_table_view.foreground_role()));
    auto data = index.data();
    auto text_alignment = index.data(GUI::ModelRole::TextAlignment).to_text_alignment(Gfx::TextAlignment::CenterLeft);
    painter.draw_text(rect, data.to_string(), m_table_view.font_for_index(index), text_alignment, text_color, Gfx::TextElision::Right);
}

}
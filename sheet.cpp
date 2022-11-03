#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
	if (!pos.IsValid()) {
		throw InvalidPositionException("Invalid position");
	}
	table_.resize(std::max(pos.row + 1, static_cast<int>(table_.size())));
	table_[pos.row].resize(std::max(pos.col + 1, static_cast<int>(table_[pos.row].size())));
	auto& cell = table_[pos.row][pos.col];
	if (!cell) {
		cell = std::make_unique<Cell>();
	}
	cell->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
	if (!pos.IsValid()) {
		throw InvalidPositionException("Invalid position");
	}
	if (pos.row >= static_cast<int>(table_.size()) ||
		pos.col >= static_cast<int>(table_[pos.row].size())) {
		return nullptr;
	}
	return table_[pos.row][pos.col].get();
}

CellInterface* Sheet::GetCell(Position pos) {
	if (!pos.IsValid()) {
		throw InvalidPositionException("Invalid position");
	}
	if (pos.row >= static_cast<int>(table_.size()) || pos.col >= static_cast<int>(table_[pos.row].size())) {
		return nullptr;
	}
	return table_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
	if (!pos.IsValid()) {
		throw InvalidPositionException("invalid position");
	}
	if (pos.row < static_cast<int>(table_.size()) &&
		pos.col < static_cast<int>(table_[pos.row].size())) {
		if (auto& cell = table_[pos.row][pos.col]) {
			cell->Clear();
			cell.reset();
		}
	}
}

Size Sheet::GetPrintableSize() const {
	Size size{ 0, 0 };
	for (int row = 0; row < static_cast<int>(table_.size()); ++row) {
		for (int col = static_cast<int>(table_[row].size()) - 1; col >= 0; --col) {
			if (const auto& cell = table_[row][col]) {
				if (!cell->GetText().empty()) {
					size.rows = std::max(size.rows, row + 1);
					size.cols = std::max(size.cols, col + 1);
					break;
				}
			}
		}
	}
	return size;
}

void Sheet::PrintValues(std::ostream& output) const {
	auto size = GetPrintableSize();
	for (int row = 0; row < size.rows; ++row) {
		int last_cell = static_cast<int>(table_[row].size());
		for (int col = 0; col < size.cols; ++col) {
			if (col > 0) {
				output << '\t';
			}
			if (col < last_cell) {
				if (const auto& cell = table_[row][col]) {
					std::visit([&output](auto&& arg) { output << arg; }, cell->GetValue());
				}
			}
		}
		output << '\n';
	}
}
void Sheet::PrintTexts(std::ostream& output) const {
	auto size = GetPrintableSize();
	for (int row = 0; row < size.rows; ++row) {
		int last_cell = static_cast<int>(table_[row].size());
		for (int col = 0; col < size.cols; ++col) {
			if (col > 0) {
				output << '\t';
			}
			if (col < last_cell) {
				if (const auto& cell = table_[row][col]) {
					output << cell->GetText();
				}
			}
		}
		output << '\n';
	}
}

std::unique_ptr<SheetInterface> CreateSheet() {
	return std::make_unique<Sheet>();
}
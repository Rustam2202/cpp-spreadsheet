#include "cell.h"
#include "formula.h"

#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

using namespace std::literals;

class Cell::Impl {
public:
	virtual Value GetValue() const = 0;
	virtual std::string GetText() const = 0;
	virtual std::vector<Position> GetReferencedCells() const { return {}; }
	virtual std::optional<FormulaInterface::Value> GetCache() const { return std::nullopt; }
	virtual void InvalidateCache() {}
};

class Cell::EmptyImpl : public Impl {
public:
	Value GetValue() const override { return {}; }
	std::string GetText() const override { return {}; }
};

class Cell::TextImpl : public Impl {
public:
	TextImpl(std::string text) : text_(std::move(text)) {
		if (text_.empty()) {
			throw std::logic_error("");
		}
	}

	Value GetValue() const override {
		return text_[0] == ESCAPE_SIGN ? text_.substr(1) : text_;
	}

	std::string GetText() const override {
		return text_;
	}

private:
	std::string text_;
};

class Cell::FormulaImpl : public Impl {
public:
	FormulaImpl(std::string expression, const SheetInterface& sheet) : sheet_(sheet) {
		if (expression.empty() || expression[0] != FORMULA_SIGN) {
			throw std::logic_error("");
		}
		formula_ = ParseFormula(expression.substr(1));
	}

	Value GetValue() const override {
		if (!cache_) {
			cache_ = formula_->Evaluate(sheet_);
		}
		return std::visit([](const auto& x) { return Value(x); }, *cache_);
	}

	std::string GetText() const override {
		return FORMULA_SIGN + formula_->GetExpression();
	}

	std::vector<Position> GetReferencedCells() const {
		return formula_->GetReferencedCells();
	}

	void InvalidateCache() override {
		cache_.reset();
	}

	std::optional<FormulaInterface::Value> GetCache() const override {
		return cache_;
	}

private:
	const SheetInterface& sheet_;
	std::unique_ptr<FormulaInterface> formula_;
	mutable std::optional<FormulaInterface::Value> cache_;
};

Cell::Cell(SheetInterface& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

Cell::~Cell() {}

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_->GetReferencedCells();
}

void Cell::Set(std::string text) {
	std::unique_ptr<Impl> impl;

	if (text.empty()) {
		impl = std::make_unique<EmptyImpl>();
	}
	else if (text.size() > 1 && text[0] == FORMULA_SIGN) {
		impl = std::make_unique<FormulaImpl>(std::move(text), sheet_);
	}
	else {
		impl = std::make_unique<TextImpl>(std::move(text));
	}

	CheckCircularDependency(impl->GetReferencedCells());
	impl_ = std::move(impl);

	UpdateDependencies();
	InvalidateCache(this);
}

void Cell::UpdateDependencies() {
	for (Cell* cell : dependent_cells_) {
		cell->referenced_cells_.erase(this);
	}
	referenced_cells_.clear();

	for (const auto& pos : impl_->GetReferencedCells()) {
		CellInterface* ref_cell = sheet_.GetCell(pos);

		if (!ref_cell) {
			sheet_.SetCell(pos, {});
			ref_cell = sheet_.GetCell(pos);
		}
		referenced_cells_.insert(static_cast<Cell*>(ref_cell));
		static_cast<Cell*>(ref_cell)->dependent_cells_.insert(this);
	}
}

void Cell::CheckCircularDependency(const std::vector<Position>& ref_cells) const {
	std::unordered_set<const CellInterface*> visited;
	CheckCircularDependencyImpl(visited, ref_cells);
}

void Cell::CheckCircularDependencyImpl(std::unordered_set<const CellInterface*>& visited,
	const std::vector<Position>& positions) const {
	for (const auto& pos : positions) {
		CellInterface* cell = sheet_.GetCell(pos);
		if (cell == this) {
			throw CircularDependencyException("");
		}
		if (cell && !visited.count(cell)) {
			const auto ref_cells = cell->GetReferencedCells();
			if (!ref_cells.empty())
				CheckCircularDependencyImpl(visited, ref_cells);
			visited.insert(cell);
		}
	}
}

void Cell::InvalidateCache(Cell* cell) {
	for (Cell* dep_cell : cell->dependent_cells_) {
		if (dep_cell->impl_->GetCache()) {
			InvalidateCache(dep_cell);
			dep_cell->impl_->InvalidateCache();
		}
	}
}

void Cell::Clear() {
	impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
	return impl_->GetValue();
}
std::string Cell::GetText() const {
	return impl_->GetText();
}
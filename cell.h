#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
	Cell(SheetInterface& sheet);
	~Cell();

	void Set(std::string text);
	void Clear();

	Value GetValue() const override;
	std::string GetText() const override;
	std::vector<Position> GetReferencedCells() const override;

	void UpdateDependencies();
	void CheckCircularDependency(const std::vector<Position>&) const;
	void CheckCircularDependencyImpl(std::unordered_set<const CellInterface*>&, const std::vector<Position>&) const;
	void InvalidateCache(Cell* cell);

private:
	class Impl;
	class EmptyImpl;
	class TextImpl;
	class FormulaImpl;

	std::unique_ptr<Impl> impl_;
	SheetInterface& sheet_;
	std::unordered_set<Cell*> referenced_cells_;
	std::unordered_set<Cell*> dependent_cells_;
};
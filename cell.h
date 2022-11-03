#pragma once

#include "common.h"
#include "formula.h"

#include <unordered_set>

class Cell : public CellInterface {
public:
	Cell();
	~Cell();

	void Set(std::string text);
	void Clear();
	Value GetValue() const override;
	std::string GetText() const override;
	std::vector<Position> GetReferencedCells();

private:
	class Impl;
	class EmptyImpl;
	class TextImpl;
	class FormulaImpl;

	SheetInterface& sheet_;
	std::unique_ptr<Impl> impl_;
	//std::unordered_set<Cell*> cells_;
};

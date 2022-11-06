#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
	Cell(const SheetInterface& sheet);
	~Cell();

	void Set(std::string text);
	void Clear();

	Value GetValue() const override;
	std::string GetText() const override;
	std::vector<Position> GetReferencedCells() const override;

	bool IsReferenced() const;

private:
	class Impl;
	class EmptyImpl;
	class TextImpl;
	class FormulaImpl;

	std::unique_ptr<Impl> impl_;
	const SheetInterface& sheet_;
};
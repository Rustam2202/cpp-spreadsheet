#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

FormulaError::FormulaError(Category category) : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
	return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
	return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
	switch (category_) {
	case Category::Ref:
		return "#REF!"sv;
	case Category::Value:
		return "#VALUE!"sv;
	case Category::Div0:
		return "#DIV/0!"sv;
	}
	return ""sv;
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
	return output << fe.ToString();
}

namespace {
	class Formula : public FormulaInterface {
	public:
		explicit Formula(std::string expression) : ast_(ParseFormulaAST(std::move(expression))) {}

		Value Evaluate(const SheetInterface& sheet) const override {
			const auto GetCellImpl = [&sheet](const Position pos) -> double {
				if (!pos.IsValid()) {
					throw FormulaError(FormulaError::Category::Ref);
				}
				const auto* cell = sheet.GetCell(pos);
				if (!cell) {
					return 0;
				}
				if (std::holds_alternative<double>(cell->GetValue())) {
					return std::get<double>(cell->GetValue());
				}

				if (std::holds_alternative<std::string>(cell->GetValue())) {
					auto value = std::get<std::string>(cell->GetValue());
					double result = 0;
					if (!value.empty()) {
						std::istringstream in(value);
						if (!(in >> result) || !in.eof()) {
							throw FormulaError(FormulaError::Category::Value);
						}
					}
					return result;
				}
				throw FormulaError(std::get<FormulaError>(cell->GetValue()));
			};

			try {
				return ast_.Execute(GetCellImpl);
			}
			catch (FormulaError& e) { return e; }
		}

		std::string GetExpression() const override {
			std::ostringstream result;
			ast_.PrintFormula(result);
			return result.str();
		}

		std::vector<Position> GetReferencedCells() const override {
			std::vector<Position> cells;
			for (auto cell : ast_.GetCells()) {
				if (cell.IsValid()) {
					cells.push_back(cell);
				}
			}
			cells.resize(std::unique(cells.begin(), cells.end()) - cells.begin());
			return cells;
		}

	private:
		const FormulaAST ast_;
	};
} // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
	try {
		return std::make_unique<Formula>(std::move(expression));
	}
	catch (...) { throw FormulaException(""); }
}
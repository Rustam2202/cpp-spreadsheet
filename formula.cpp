#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
	return output << "#DIV/0!";
}

namespace {
	class Formula : public FormulaInterface {
	public:
		// Реализуйте следующие методы:
		explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {}

		//Value Evaluate() const override {
		//	Value val;
		//	try {
		//		val=ast_.Execute();
		//	}
		//	catch (FormulaError& err) {
		//		val = err;
		//	}
		//	return val;
		//}

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
			std::ostringstream oss;
			ast_.PrintFormula(oss);
			return oss.str();
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
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
	//return std::make_unique<Formula>(std::move(expression));
	try {
		return std::make_unique<Formula>(std::move(expression));
	}
	catch (...) { throw FormulaException(""); }
}
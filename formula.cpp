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
		explicit Formula(std::string expression) : ast_(ParseFormulaAST(expression)) {	}

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

		}

		std::string GetExpression() const override {
			std::ostringstream oss;
			ast_.PrintFormula(oss);
			return oss.str();
		}

	private:
		FormulaAST ast_;
	};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
	return std::make_unique<Formula>(std::move(expression));
}
#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>

class Cell::Impl {
public:
	virtual Value GetValue() const = 0;
	virtual std::string GetText() const = 0;
	virtual std::vector<Position> GetReferencedCells() const {
		return {};
	}
};

class Cell::EmptyImpl :public Impl {
public:
	Value GetValue() const override {
		return {};
	}
	std::string GetText() const override {
		return {};
	}
	std::vector<Position> GetReferencedCells() const override {
		return {};
	}
};

class Cell::TextImpl :public Impl {
public:
	TextImpl(std::string text) :text_(text) {}
	Value GetValue() const override {
		return text_.at(0) == ESCAPE_SIGN ? text_.substr(1) : text_;
	}
	std::string GetText() const override {
		return text_;
	}
private:
	std::string text_;
};

class Cell::FormulaImpl :public Impl {
public:
	//FormulaImpl(std::string text)
	//	:  text_(text), formula_(ParseFormula(text.substr(1))) {
	//}

	FormulaImpl(std::string expression, const SheetInterface& sheet) : sheet_(sheet) {
		/*if (expression.empty() || expression[0] != FORMULA_SIGN) {
			throw std::logic_error("");
		}*/
		formula_ = ParseFormula(expression.substr(1));
	}

	Value GetValue() const override {
		/*std::variant<double, FormulaError> result = formula_->Evaluate();
		if (auto double_answ = std::get_if<double>(&result)) {
			return *double_answ;
		}
		else if (auto err_answ = std::get_if<FormulaError>(&result)) {
			return *err_answ;
		}
		return {};*/

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

private:
	//std::string text_;
	const SheetInterface& sheet_;
	std::unique_ptr<FormulaInterface> formula_;
	mutable std::optional<FormulaInterface::Value> cache_;
};

//Cell::Cell() : impl_(std::make_unique<Cell::EmptyImpl>()) {}
Cell::Cell(const SheetInterface& sheet) :
	impl_(std::make_unique<EmptyImpl>()),
	sheet_(sheet) 
{}

Cell::~Cell() {}

void Cell::Set(std::string text) {
	if (text.empty()) {
		impl_ = std::make_unique<EmptyImpl>();
	}
	else if (text.size() > 1 && text.at(0) == FORMULA_SIGN) {
		impl_ = std::make_unique<FormulaImpl>(text, sheet_);
	}
	else {
		impl_ = std::make_unique<TextImpl>(text);
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

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
	return true;
}

#pragma once

class ExpressionParser {

	public:
		ExpressionParser(const std::string &expr);
		~ExpressionParser();
		void tokenizeExpr();
		double eval();

	private:
		struct token {
			char c;
			double d;
			bool isNum;
			bool isOp;
			token() : isNum(false), isOp(true), c(0) {};
		};
		const std::string m_expr;
		std::vector<token> tokens;
		std::stack<token> m_opstack;
		std::queue<token> m_revpol;
		std::stack<double> m_output;
		void shuntingYard();
		void evalRevPol();

};
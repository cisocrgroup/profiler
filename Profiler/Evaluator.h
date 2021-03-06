#ifndef OCRCorrection_Evaluator_hpp__
#define OCRCorrection_Evaluator_hpp__

#include <sys/stat.h>
#include <sys/types.h>
#include "Document/Document.h"
#include <array>

namespace OCRCorrection {
	class GlobalProfile;

	class Evaluator {
	public:
		enum class Class {
			TruePositive,
			TrueNegative,
			FalsePositive,
			FalseNegativeObjective,
			FalseNegativeFair,
		};
		enum class Mode {
			Normal,
			Strict,
			VeryStrict,
		};
		Evaluator(): classes_(), mode_(Mode::Normal) {}
		const std::vector<size_t>& operator[](Class c) const noexcept {
			return classes_[static_cast<size_t>(c)];
		}
		size_t true_positives() const noexcept {
			return (*this)[Class::TruePositive].size();
		}
		size_t true_negatives() const noexcept {
			return (*this)[Class::TrueNegative].size();
		}
		size_t false_positives() const noexcept {
			return (*this)[Class::FalsePositive].size();
		}
		size_t false_negatives_fair() const noexcept {
			return (*this)[Class::FalseNegativeFair].size();
		}
		size_t false_negatives_objective() const noexcept {
			return (*this)[Class::FalseNegativeObjective].size() +
				false_negatives_fair();
		}
		void clear() noexcept {
			std::fill(begin(classes_), end(classes_),
					std::vector<size_t>());
		}
		void set_mode(Mode mode) {mode_ = mode;}

		double precision() const noexcept;
		double recall_fair() const noexcept;
		double recall_objective() const noexcept;

		void write(const std::string& path, const Document& doc) const;
		void classify(const Document& doc);
		void classify(const Token& token);

	private:
		struct ModeNormal{};
		struct ModeStrict{};
		struct ModeVeryStrict{};
		struct CandidateRange {
			CandidateRange(const Token& token)
				: b_(token.candidatesBegin())
				, e_(token.candidatesEnd())
			{}
			Token::CandidateIterator begin() const noexcept {return b_;}
			Token::CandidateIterator end() const noexcept {return e_;}
			bool empty() const noexcept {return b_ == e_;}
			Token::CandidateIterator b_, e_;
		};
		Class get_class(const Token& token) const;
		template<class M>
		static Class get_class(const Token& token, M);
		static Class get_false_negative(const Token& token);
		static bool is_true_positive(const Token& token, ModeNormal);
		static bool is_true_positive(const Token& token, ModeStrict);
		static bool is_true_positive(const Token& token, ModeVeryStrict);

		void write(std::wostream& os, Class c, const Document& doc) const;
		void write(std::wostream& os, const GlobalProfile& gp) const;
		std::vector<size_t>& operator[](Class c) noexcept {
			return classes_[static_cast<size_t>(c)];
		}
		static bool has_ocr_errors(const Token& token);

		std::array<std::vector<size_t>, 5> classes_;
		std::vector<size_t> unknowns_;
		Mode mode_;
		int ntest_, neval_, nx_;
	};
}

////////////////////////////////////////////////////////////////////////////////
template<class M>
OCRCorrection::Evaluator::Class
OCRCorrection::Evaluator::get_class(const Token& token, M m)
{
	if (token.metadata()["groundtruth-lc"] != token.getWOCR_lc()) {
		if (has_ocr_errors(token)) {
			if (is_true_positive(token, m)) {
				// std::wcout << token.metadata()["groundtruth-lc"] << ":"
				// 	   << token.getWOCR_lc() << ":"
				// 	   << token.metadata()["touch"]
				// 	   << "\n";
				// using std::begin;
				// using std::end;
				// CandidateRange r(token);
				// for (const auto& c: r) {
				// 	std::wcout << "cand:" << c << "\n";
				// }
				assert(token.metadata()["touch"] == L"true");
				return Class::TruePositive;
			} else {
				return get_false_negative(token);
			}
		} else {
			return get_false_negative(token);
		}
	} else {
		if (has_ocr_errors(token)) {
			// std::wcout << token.metadata()["groundtruth-lc"] << ":"
			// 	   << token.getWOCR_lc() << ":"
			// 	   << token.metadata()["touch"]
			// 	   << "\n";
			// using std::begin;
			// using std::end;
			// CandidateRange r(token);
			// for (const auto& c: r) {
			// 	std::wcout << "cand:" << c << "\n";
			// }
			assert(token.metadata()["touch"] == L"true");
			return Class::FalsePositive;
		} else {
			return Class::TrueNegative;
		}
	}
}

#endif // OCRCorrection_Evaluator_hpp__

#ifndef utils_h
#define utils_h

#include <Eigen/Core>
#include <Eigen/LU>
#include <cassert>
#include <cmath>
#include <string>

namespace sequential_line_search
{
    namespace utils
    {
        ////////////////////////////////////////////////
        // Random
        ////////////////////////////////////////////////

        // Uniform sampling from [0, 1]^n
        Eigen::VectorXd generateRandomVector(unsigned n);

        ////////////////////////////////////////////////
        // Bradley-Terry-Luce Model
        ////////////////////////////////////////////////

        inline double CalcBtl(const Eigen::VectorXd& f, double scale = 1.0)
        {
            const unsigned dim = f.rows();
            double         sum = 0.0;
            for (unsigned i = 0; i < dim; ++i)
            {
                sum += std::exp(f(i) / scale);
            }
            return std::exp(f(0) / scale) / sum;
        }

        inline Eigen::VectorXd CalcBtlDerivative(const Eigen::VectorXd& f, double scale = 1.0)
        {
            const unsigned dim = f.rows();
            const double   btl = CalcBtl(f, scale);
            const double   tmp = -btl * btl / scale;

            Eigen::VectorXd d(dim);

            double sum = 0.0;
            for (unsigned i = 1; i < dim; ++i)
            {
                sum += std::exp((f(i) - f(0)) / scale);
            }
            d(0) = -sum;

            for (unsigned i = 1; i < dim; ++i)
            {
                d(i) = std::exp((f(i) - f(0)) / scale);
            }

            return tmp * d;
        }

        ////////////////////////////////////////////////
        // File IO
        ////////////////////////////////////////////////

        void exportMatrixToCsv(const std::string& filePath, const Eigen::MatrixXd& X);
    } // namespace utils
} // namespace sequential_line_search

#endif // utils_h
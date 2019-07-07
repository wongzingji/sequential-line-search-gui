#include <sequential-line-search/acquisition-function.h>
#include <sequential-line-search/data.h>
#include <sequential-line-search/preference-regressor.h>
#include <sequential-line-search/sequential-line-search.h>
#include <sequential-line-search/slider.h>
#include <sequential-line-search/utils.h>
#include <stdexcept>

sequential_line_search::SequentialLineSearchOptimizer::SequentialLineSearchOptimizer(const int  dimension,
                                                                                     const bool use_slider_enlargement,
                                                                                     const bool use_MAP_hyperparameters)
    : m_dimension(dimension), m_use_slider_enlargement(use_slider_enlargement),
      m_use_MAP_hyperparameters(use_MAP_hyperparameters), m_a(0.500), m_r(0.500), m_b(0.005), m_variance(0.250),
      m_btl_scale(0.010)
{
    m_data      = std::make_shared<Data>();
    m_regressor = nullptr;
    m_slider    = std::make_shared<Slider>(
        utils::generateRandomVector(m_dimension), utils::generateRandomVector(m_dimension), false);
}

void sequential_line_search::SequentialLineSearchOptimizer::setHyperparameters(
    const double a, const double r, const double b, const double variance, const double btl_scale)
{
    m_a         = a;
    m_r         = r;
    m_b         = b;
    m_variance  = variance;
    m_btl_scale = btl_scale;
}

void sequential_line_search::SequentialLineSearchOptimizer::submit(const double slider_position)
{
    const auto  x_chosen   = getParameters(slider_position);
    const auto& x_prev_max = m_slider->orig_0;
    const auto& x_prev_ei  = m_slider->orig_1;

    m_data->AddNewPoints(x_chosen, {x_prev_max, x_prev_ei}, true);

    m_regressor = std::make_shared<PreferenceRegressor>(
        m_data->X, m_data->D, Eigen::VectorXd(), m_use_MAP_hyperparameters, m_a, m_r, m_b, m_variance, m_btl_scale);

    const auto x_max = m_regressor->find_arg_max();
    const auto x_ei  = acquisition_function::FindNextPoint(*m_regressor);

    m_slider = std::make_shared<Slider>(x_max, x_ei, m_use_slider_enlargement);
}

std::pair<Eigen::VectorXd, Eigen::VectorXd> sequential_line_search::SequentialLineSearchOptimizer::getSliderEnds() const
{
    return {m_slider->end_0, m_slider->end_1};
}

Eigen::VectorXd sequential_line_search::SequentialLineSearchOptimizer::getParameters(const double slider_position) const
{
    return m_slider->getValue(slider_position);
}

Eigen::VectorXd sequential_line_search::SequentialLineSearchOptimizer::getMaximizer() const { return m_slider->orig_0; }

double
sequential_line_search::SequentialLineSearchOptimizer::getPreferenceValueMean(const Eigen::VectorXd& parameter) const
{
    return (m_regressor == nullptr) ? 0.0 : m_regressor->estimate_y(parameter);
}

double sequential_line_search::SequentialLineSearchOptimizer::getPreferenceValueStandardDeviation(
    const Eigen::VectorXd& parameter) const
{
    return (m_regressor == nullptr) ? 0.0 : m_regressor->estimate_s(parameter);
}

double sequential_line_search::SequentialLineSearchOptimizer::getExpectedImprovementValue(
    const Eigen::VectorXd& parameter) const
{
    return (m_regressor == nullptr) ? 0.0 : acquisition_function::CalculateAcqusitionValue(*m_regressor, parameter);
}

const Eigen::MatrixXd& sequential_line_search::SequentialLineSearchOptimizer::getRawDataPoints() const
{
    return m_data->X;
}

void sequential_line_search::SequentialLineSearchOptimizer::dampData(const std::string& directory_path) const
{
    if (m_regressor == nullptr)
    {
        return;
    }

    m_regressor->dampData(directory_path);
}

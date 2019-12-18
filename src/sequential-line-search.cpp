#include <sequential-line-search/acquisition-function.hpp>
#include <sequential-line-search/preference-data-manager.hpp>
#include <sequential-line-search/preference-regressor.hpp>
#include <sequential-line-search/sequential-line-search.hpp>
#include <sequential-line-search/slider.hpp>
#include <sequential-line-search/utils.hpp>
#include <stdexcept>

std::pair<Eigen::VectorXd, Eigen::VectorXd> sequential_line_search::GenerateRandomSliderEnds(const int num_dims)
{
    return {0.5 * (Eigen::VectorXd::Random(num_dims) + Eigen::VectorXd::Ones(num_dims)),
            0.5 * (Eigen::VectorXd::Random(num_dims) + Eigen::VectorXd::Ones(num_dims))};
}

std::pair<Eigen::VectorXd, Eigen::VectorXd>
sequential_line_search::GenerateCenteredFixedLengthRandomSliderEnds(const int num_dims)
{
    const Eigen::VectorXd x_center   = Eigen::VectorXd::Constant(num_dims, 0.50);
    const Eigen::VectorXd random_dir = 0.5 * Eigen::VectorXd::Random(num_dims);

    return {x_center + random_dir, x_center - random_dir};
}

sequential_line_search::SequentialLineSearchOptimizer::SequentialLineSearchOptimizer(
    const int                                                                    num_dims,
    const bool                                                                   use_slider_enlargement,
    const bool                                                                   use_map_hyperparams,
    const std::function<std::pair<Eigen::VectorXd, Eigen::VectorXd>(const int)>& initial_slider_generator)
    : m_use_slider_enlargement(use_slider_enlargement),
      m_use_map_hyperparams(use_map_hyperparams),
      m_kernel_signal_var(0.500),
      m_kernel_length_scale(0.500),
      m_noise_level(0.005),
      m_kernel_hyperparams_prior_var(0.250),
      m_btl_scale(0.010)
{
    const auto slider_ends = initial_slider_generator(num_dims);

    m_data      = std::make_shared<PreferenceDataManager>();
    m_regressor = nullptr;
    m_slider    = std::make_shared<Slider>(std::get<0>(slider_ends), std::get<1>(slider_ends), false);
}

void sequential_line_search::SequentialLineSearchOptimizer::SetHyperparams(const double kernel_signal_var,
                                                                           const double kernel_length_scale,
                                                                           const double noise_level,
                                                                           const double kernel_hyperparams_prior_var,
                                                                           const double btl_scale)
{
    m_kernel_signal_var            = kernel_signal_var;
    m_kernel_length_scale          = kernel_length_scale;
    m_noise_level                  = noise_level;
    m_kernel_hyperparams_prior_var = kernel_hyperparams_prior_var;
    m_btl_scale                    = btl_scale;
}

void sequential_line_search::SequentialLineSearchOptimizer::SubmitLineSearchResult(const double slider_position)
{
    // A heuristics to set the computational effort for solving the maximization of the acquisition function. This is
    // not justified or validated.
    const int num_dims                 = GetMaximizer().size();
    const int num_map_estimation_iters = 100;
    const int num_global_search_iters  = 50 * num_dims;
    const int num_local_search_iters   = 10 * num_dims;

    SubmitLineSearchResult(slider_position, num_map_estimation_iters, num_global_search_iters, num_local_search_iters);
}

void sequential_line_search::SequentialLineSearchOptimizer::SubmitLineSearchResult(const double slider_position,
                                                                                   const int num_map_estimation_iters,
                                                                                   const int num_global_search_iters,
                                                                                   const int num_local_search_iters)
{
    const auto  x_chosen   = CalcPointFromSliderPosition(slider_position);
    const auto& x_prev_max = m_slider->original_end_0;
    const auto& x_prev_ei  = m_slider->original_end_1;

    // Update the data
    m_data->AddNewPoints(x_chosen, {x_prev_max, x_prev_ei}, true);

    // Perform the MAP estimation
    m_regressor = std::make_shared<PreferenceRegressor>(m_data->m_X,
                                                        m_data->m_D,
                                                        m_use_map_hyperparams,
                                                        m_kernel_signal_var,
                                                        m_kernel_length_scale,
                                                        m_noise_level,
                                                        m_kernel_hyperparams_prior_var,
                                                        m_btl_scale,
                                                        num_map_estimation_iters);

    // Find the next search subspace
    const auto x_max = m_regressor->FindArgMax();
    const auto x_ei =
        acquisition_function::FindNextPoint(*m_regressor, num_global_search_iters, num_local_search_iters);

    m_slider = std::make_shared<Slider>(x_max, x_ei, m_use_slider_enlargement);
}

std::pair<Eigen::VectorXd, Eigen::VectorXd> sequential_line_search::SequentialLineSearchOptimizer::GetSliderEnds() const
{
    return {m_slider->end_0, m_slider->end_1};
}

Eigen::VectorXd
sequential_line_search::SequentialLineSearchOptimizer::CalcPointFromSliderPosition(const double slider_position) const
{
    return m_slider->GetValue(slider_position);
}

Eigen::VectorXd sequential_line_search::SequentialLineSearchOptimizer::GetMaximizer() const
{
    return m_slider->original_end_0;
}

double sequential_line_search::SequentialLineSearchOptimizer::GetPreferenceValueMean(const Eigen::VectorXd& point) const
{
    return (m_regressor == nullptr) ? 0.0 : m_regressor->PredictMu(point);
}

double
sequential_line_search::SequentialLineSearchOptimizer::GetPreferenceValueStdev(const Eigen::VectorXd& point) const
{
    return (m_regressor == nullptr) ? 0.0 : m_regressor->PredictSigma(point);
}

double
sequential_line_search::SequentialLineSearchOptimizer::GetExpectedImprovementValue(const Eigen::VectorXd& point) const
{
    return (m_regressor == nullptr) ? 0.0 : acquisition_function::CalcAcqusitionValue(*m_regressor, point);
}

const Eigen::MatrixXd& sequential_line_search::SequentialLineSearchOptimizer::GetRawDataPoints() const
{
    return m_data->m_X;
}

void sequential_line_search::SequentialLineSearchOptimizer::DampData(const std::string& directory_path) const
{
    if (m_regressor == nullptr)
    {
        return;
    }

    m_regressor->DampData(directory_path);
}

#include "TestPatternSource.h"

#include <algorithm>
#include <chrono>

#include <opencv2/imgproc.hpp>

namespace pivision::capture {

TestPatternSource::TestPatternSource(const TestPatternConfig &config)
    : m_config(config)
{
}

bool TestPatternSource::open()
{
    if (m_config.width <= 0 || m_config.height <= 0 || m_config.fps <= 0.0)
        return false;

    // Horizontal gradient, built once and copied into every frame.
    m_background.create(m_config.height, m_config.width, CV_8UC3);
    for (int x = 0; x < m_config.width; ++x) {
        const double t = static_cast<double>(x) / m_config.width;
        m_background.col(x).setTo(cv::Scalar(90 + 60 * t, 40 + 40 * t, 30));
    }
    m_index = 0;
    return true;
}

bool TestPatternSource::read(Frame &out)
{
    if (m_background.empty())
        return false;

    m_background.copyTo(out.image); // reuses out.image's buffer when the size matches

    const int box = std::max(1, m_config.height / 4);
    const int travel = std::max(1, m_config.width - box);
    const int x = static_cast<int>((m_index * 4) % static_cast<std::uint64_t>(travel));
    const int y = (m_config.height - box) / 2;
    cv::rectangle(out.image, cv::Rect(x, y, box, box), cv::Scalar(60, 200, 255), cv::FILLED);
    cv::putText(out.image, std::to_string(m_index), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX,
        0.8, cv::Scalar(255, 255, 255), 2);

    out.index = m_index++;
    out.captured = std::chrono::steady_clock::now();
    return true;
}

void TestPatternSource::close() { m_background.release(); }

std::string TestPatternSource::name() const { return "Test pattern"; }

double TestPatternSource::nominalFps() const { return m_config.fps; }

} // namespace pivision::capture

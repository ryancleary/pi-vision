#include <gtest/gtest.h>

#include <opencv2/core.hpp>

#include <pivision/utils/Images.h>

namespace pivision::utils {

TEST(ImagesGTest, ColorMatBecomesBgrImageWithSamePixels)
{
    const cv::Mat mat(4, 6, CV_8UC3, cv::Scalar(10, 20, 30)); // B, G, R
    const QImage image = toQImage(mat);
    EXPECT_EQ(image.size(), QSize(6, 4));
    EXPECT_EQ(image.format(), QImage::Format_BGR888);
    EXPECT_EQ(image.pixelColor(0, 0), QColor(30, 20, 10));
}

TEST(ImagesGTest, GrayMatBecomesGrayImage)
{
    const cv::Mat mat(4, 6, CV_8UC1, cv::Scalar(77));
    const QImage image = toQImage(mat);
    EXPECT_EQ(image.format(), QImage::Format_Grayscale8);
    EXPECT_EQ(qGray(image.pixel(5, 3)), 77);
}

TEST(ImagesGTest, ImageOwnsItsPixels)
{
    cv::Mat mat(2, 2, CV_8UC1, cv::Scalar(5));
    const QImage image = toQImage(mat);
    mat.setTo(cv::Scalar(200));
    EXPECT_EQ(qGray(image.pixel(0, 0)), 5);
}

} // namespace pivision::utils

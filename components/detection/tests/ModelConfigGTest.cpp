#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <pivision/detection/ModelConfig.h>

namespace pivision::detection {

class ModelConfigTest : public ::testing::Test {
protected:
    // The model description under test.
    static constexpr const char *kModelDescription = "nanodet-plus-m-1.5x-416.json";

    static std::string nanoDetDescription()
    {
        return std::string(PIVISION_MODEL_DIR) + "/" + kModelDescription;
    }

    static std::string readFile(const std::string &path)
    {
        std::ifstream in(path);
        std::stringstream text;
        text << in.rdbuf();
        return text.str();
    }

    // Writes the NanoDet description with `from` replaced by `to` to a
    // temporary file, loads it, and returns the error (empty if it loaded).
    static std::string errorAfterReplacing(const std::string &from, const std::string &to)
    {
        std::string text = readFile(nanoDetDescription());
        const auto at = text.find(from);
        EXPECT_NE(at, std::string::npos) << "test setup: \"" << from << "\" not in the file";
        text.replace(at, from.size(), to);

        const auto path = std::filesystem::path(::testing::TempDir()) / "pivision-model.json";
        std::ofstream(path) << text;
        std::string error;
        const bool loaded = loadModelConfig(path.string(), &error).has_value();
        EXPECT_EQ(loaded, error.empty());
        return error;
    }
};

TEST_F(ModelConfigTest, GivenTheNanoDetDescription_WhenLoading_ThenItsValuesAreRead)
{
    std::string error;
    const auto config = loadModelConfig(nanoDetDescription(), &error);
    ASSERT_TRUE(config.has_value()) << error;

    // Spot checks: each expected value is copied from the description file,
    // which also says what it means and where it comes from.
    EXPECT_EQ(config->inputSize, cv::Size(416, 416));                // input.size
    EXPECT_EQ(config->channelOrder, ChannelOrder::Bgr);              // input.channelOrder
    EXPECT_EQ(config->resize, ResizeMode::Letterbox);                // input.resize
    EXPECT_EQ(config->strides, (std::vector<int> { 8, 16, 32 }));    // output.strides
    EXPECT_EQ(config->classes.size(), 80U);                          // the 80 COCO classes
    EXPECT_EQ(config->classes.front(), "person");                    // first class
    EXPECT_EQ(config->classes.back(), "toothbrush");                 // last class
}

TEST_F(ModelConfigTest, GivenADescription_WhenLoading_ThenTheModelFileIsResolvedNextToIt)
{
    const auto config = loadModelConfig(nanoDetDescription());
    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(std::filesystem::path(config->modelPath).parent_path(),
        std::filesystem::path(nanoDetDescription()).parent_path());
}

TEST_F(ModelConfigTest, GivenAMissingFile_WhenLoading_ThenAnErrorIsReported)
{
    std::string error;
    EXPECT_FALSE(loadModelConfig("/nonexistent/model.json", &error).has_value());
    EXPECT_FALSE(error.empty());
}

TEST_F(ModelConfigTest, GivenAnUnknownChannelOrder_WhenLoading_ThenAnErrorNamesIt)
{
    EXPECT_NE(errorAfterReplacing("\"value\": \"BGR\"", "\"value\": \"BRG\"").find("channelOrder"),
        std::string::npos);
}

TEST_F(ModelConfigTest, GivenAnUnknownResizeMode_WhenLoading_ThenAnErrorNamesIt)
{
    EXPECT_NE(errorAfterReplacing("\"value\": \"letterbox\"", "\"value\": \"crop\"").find("resize"),
        std::string::npos);
}

TEST_F(ModelConfigTest, GivenAMeanWithTwoValues_WhenLoading_ThenAnErrorNamesIt)
{
    // input.mean as written in the file, and the same with its last value dropped.
    const std::string mean = "[103.53, 116.28, 123.675]";
    const std::string twoValues = "[103.53, 116.28]";
    EXPECT_NE(errorAfterReplacing(mean, twoValues).find("mean"), std::string::npos);
}

TEST_F(ModelConfigTest, GivenBrokenJson_WhenLoading_ThenAnErrorIsReported)
{
    EXPECT_NE(errorAfterReplacing("\"name\":", "\"name\"").find("JSON"), std::string::npos);
}

} // namespace pivision::detection

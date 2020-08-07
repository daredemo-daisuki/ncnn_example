#include "insightface.h"
#include <iostream>
#include <string>

#if MIRROR_VULKAN
#include "gpu.h"
#endif // MIRROR_VULKAN

namespace mirror {
InsightfaceLandmarker::InsightfaceLandmarker() {
	insightface_landmarker_net_ = new ncnn::Net();
	initialized = false;
#if MIRROR_VULKAN
	ncnn::create_gpu_instance();	
    insightface_landmarker_net_->opt.use_vulkan_compute = true;
#endif // MIRROR_VULKAN
}

InsightfaceLandmarker::~InsightfaceLandmarker() {
	insightface_landmarker_net_->clear();
#if MIRROR_VULKAN
	ncnn::destroy_gpu_instance();
#endif // MIRROR_VULKAN	
}

int InsightfaceLandmarker::LoadModel(const char * root_path) {
	std::string fl_param = std::string(root_path) + "/2d106.param";
	std::string fl_bin = std::string(root_path) + "/2d106.bin";
	if (insightface_landmarker_net_->load_param(fl_param.c_str()) == -1 ||
		insightface_landmarker_net_->load_model(fl_bin.c_str()) == -1) {
		std::cout << "load face landmark model failed." << std::endl;
		return 10000;
	}
	initialized = true;
	return 0;
}

int InsightfaceLandmarker::ExtractKeypoints(const cv::Mat & img_src,
	const cv::Rect & face, std::vector<cv::Point2f>* keypoints) {
	std::cout << "start extract keypoints." << std::endl;
	keypoints->clear();
	if (!initialized) {
		std::cout << "insightface landmarker unitialized." << std::endl;
		return 10000;
	}

	if (img_src.empty()) {
		std::cout << "input empty." << std::endl;
		return 10001;
	}

	cv::Mat img_face = img_src(face).clone();
	ncnn::Extractor ex = insightface_landmarker_net_->create_extractor();
	ncnn::Mat in = ncnn::Mat::from_pixels_resize(img_face.data,
		ncnn::Mat::PIXEL_BGR, img_face.cols, img_face.rows, 192, 192);
	ex.input("data", in);
	ncnn::Mat out;
	ex.extract("fc1", out);

	for (int i = 0; i < 106; ++i) {
		float x = (out[2 * i] + 1.0f) * img_face.cols / 2 + face.x;
		float y = (out[2 * i + 1] + 1.0f) * img_face.rows / 2 + face.y;
		keypoints->push_back(cv::Point2f(x, y));
	}


	std::cout << "end extract keypoints." << std::endl;
	return 0;
}

}

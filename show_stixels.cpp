/**
    This file is part of show_stixels. (https://github.com/dhernandez0/show_stixels).

    Copyright (c) 2016 Daniel Hernandez Juarez.

    show_stixels is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    show_stixels is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with show_stixels.  If not, see <http://www.gnu.org/licenses/>.

**/

#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <opencv2/opencv.hpp>
#include <dirent.h>
#include "vDisparity.h"

#define GROUND	0
#define OBJECT	1
#define SKY		2

struct Section {
	int u;
	int type;
	int vB, vT;
	float disparity;
};

#define OVERWRITE	true

std::vector< std::vector<struct Section> > ReadStixels(const char *fname) {
	std::ifstream fp;
	fp.open (fname, std::ofstream::in);
	std::vector< std::vector<struct Section> > res;
	if(fp.is_open()) {
		std::string line;
		while ( std::getline (fp,line) ) {
			std::vector<struct Section> sections_vec;
			char *ostr = (char *) line.c_str();
			char *str = (char *) line.c_str();
			std::vector<size_t> secs;
			for(size_t i = 0; i < line.size(); i++) {
				if(str[i] == ';') {
					secs.push_back(i);
				}
			}
			for(size_t i = 0; i < secs.size(); i++) {
				struct Section sec;
				ostr[secs.at(i)] = '\0';

				if(sscanf (str, "%d,%d,%d,%f", &sec.type, &sec.vB, &sec.vT, &sec.disparity) == 4) {
					sections_vec.push_back(sec);
				}
				if(secs.at(i)+1 < line.size()) {
					str = &ostr[secs.at(i)+1];
				}
			}
			res.push_back(sections_vec);
		}
		fp.close();
	}
	return res;
}

std::vector< std::vector<struct Section> > ReadStixelsGT(const char *fname) {
	std::ifstream fp;
	fp.open (fname, std::ofstream::in);
	std::vector< std::vector<struct Section> > res;
	if(fp.is_open()) {
		std::string line;
		while ( std::getline (fp,line) ) {
			std::vector<struct Section> sections_vec;
			char *ostr = (char *) line.c_str();
			char *str = (char *) line.c_str();
			std::vector<size_t> secs;
			for(size_t i = 0; i < line.size(); i++) {
				if(str[i] == ';') {
					secs.push_back(i);
				}
			}
			for(size_t i = 0; i < secs.size(); i++) {
				struct Section sec;
				ostr[secs.at(i)] = '\0';

				if(sscanf (str, "%d,%d,%d,%f", &sec.u, &sec.vB, &sec.vT, &sec.disparity) == 4) {
					sections_vec.push_back(sec);
				}
				if(secs.at(i)+1 < line.size()) {
					str = &ostr[secs.at(i)+1];
				}
			}
			res.push_back(sections_vec);
		}
		fp.close();
	}
	return res;
}

void HSV_to_RGB(const float h, const float s, const float v, int *cr, int *cg, int *cb) {
	const float h_prima = h*360.0f/60.0f;

	const float c = v*s;
	const float h_mod = fmodf(h_prima, 2.0f);
	const float x = c*(1.0f-fabsf(h_mod - 1.0f));

	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;

	if(h_prima >= 0) {
		if(h_prima < 1.0f) {
			r = c;
			g = x;
			b = 0.0f;
		} else if(h_prima < 2.0f) {
			r = x;
			g = c;
			b = 0.0f;
		} else if(h_prima < 3.0f) {
			r = 0.0f;
			g = c;
			b = x;
		} else if(h_prima < 4.0f) {
			r = 0.0f;
			g = x;
			b = c;
		} else if(h_prima < 5.0f) {
			r = x;
			g = 0.0f;
			b = c;
		} else if(h_prima < 6.0f) {
			r = c;
			g = 0.0f;
			b = x;
		}
	}

	const float m = v-c;
	r = (r+m)*255.0f;
	g = (g+m)*255.0f;
	b = (b+m)*255.0f;
	*cr = (int) r;
	*cg = (int) g;
	*cb = (int) b;

}

int GetHorizonRow(const char *disparity, const int max_dis) {
	cv::Mat dis = cv::imread(disparity, cv::IMREAD_UNCHANGED);

	const int rows = dis.rows;
	const int cols = dis.cols;

	// UINT16 -> FLOAT
	cv::Mat disparity_float(dis.rows, dis.cols, CV_32FC1);
	for(int i = 0; i < dis.rows; i++) {
		for(int j = 0; j < dis.cols; j++) {
			const float d = (float) dis.at<uint16_t>(i, j)/256.0f;
			disparity_float.at<float>(i, j) = d;
		}
	}

	const float focal = 704.7082f;
	const float baseline = 0.8f;
	const float camera_center_y = 384.0f;

	CvDisparity vDisp;
	vDisp.Initialize(camera_center_y, baseline, focal, max_dis);
	vDisp.Compute(disparity_float.ptr<float>(), rows, cols);

	// Get Camera Parameters
	const int vhor = vDisp.GetHorizonPoint();

	vDisp.Finish();

	return vhor;
}

bool FileExists(const char *fname) {
	struct stat buffer;
	return (stat (fname, &buffer) == 0);
}

bool directory_exists(const char* dir) {
	DIR* d = opendir(dir);
	bool ok = false;
	if(d) {
	    closedir(d);
	    ok = true;
	}
	return ok;
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        std::cerr << "Usage: show_stixels dir max_disparity" << std::endl;
		return -1;
	}
	const int column_size = 5;
	const int width_margin = 0;
	const char* directory = argv[1];
	const int max_dis = atoi(argv[2]);
	const float max_dis_display = (float) 30;
	const char* left_dir = "left";
	const char* disparity_dir = "disparities";
	const char* stixels_dir = "stixels";
	const char* stixelsim_dir = "stixelsim";

	DIR *dp;
	struct dirent *ep;
	char abs_stixelim_dir[PATH_MAX];
	sprintf(abs_stixelim_dir, "%s/%s", directory, stixelsim_dir);
	dp = opendir(abs_stixelim_dir);
	if (dp == NULL) {
		std::cerr << "Invalid directory: " << abs_stixelim_dir << std::endl;
		exit(EXIT_FAILURE);
	}
	char abs_stixel_dir[PATH_MAX];
	sprintf(abs_stixel_dir, "%s/%s", directory, stixels_dir);
	dp = opendir(abs_stixel_dir);
	if (dp == NULL) {
		std::cerr << "Invalid directory: " << abs_stixel_dir << std::endl;
		exit(EXIT_FAILURE);
	}
	char stixelim_file[PATH_MAX];
	char stixel_file[PATH_MAX];
	char disparity_file[PATH_MAX];
	char left_file[PATH_MAX];
	char filename[PATH_MAX];

	int cnt = 0;
	while ((ep = readdir(dp)) != NULL) {
		if (!strcmp (ep->d_name, "."))
			continue;
		if (!strcmp (ep->d_name, ".."))
			continue;

		// Remove .stixels
		const char *found = strchr(ep->d_name, '.');
		if(found == NULL) {
			continue;
		}

		//std::cout << ep->d_name << std::endl;
		cnt++;

		const int new_len = found-ep->d_name+1;
		memcpy(filename, ep->d_name, sizeof(char)*new_len);
		filename[new_len-1] = '\0';

		sprintf(left_file, "%s/%s/%s.%s", directory, left_dir, filename, "png");
		sprintf(stixelim_file, "%s/%s/%s.%s", directory, stixelsim_dir, filename, "png");
		sprintf(disparity_file, "%s/%s/%s.%s", directory, disparity_dir, filename, "png");
		sprintf(stixel_file, "%s/%s/%s", directory, stixels_dir, ep->d_name);
		if(FileExists(stixelim_file) && !OVERWRITE) {
			continue;
		}

		cv::Mat frame = cv::imread(left_file);

		std::vector< std::vector<Section> > stixels = ReadStixels(stixel_file);
		int horizon_row = -1;
		if(FileExists(disparity_file)) {
			horizon_row = GetHorizonRow(disparity_file, max_dis);
		}

		for(size_t i = 0; i < stixels.size(); i++) {
			std::vector<Section> column = stixels.at(i);
			Section prev;
			prev.type = -1;
			bool have_prev = false;
			for(size_t j = 0; j < column.size(); j++) {
				Section sec = column.at(j);
				sec.vB = frame.rows-1-sec.vB;
				sec.vT = frame.rows-1-sec.vT;

				// If disparity is 0 it is sky
				if(sec.type == OBJECT && sec.disparity < 1.0f) {
					sec.type = SKY;
				}
					
				// Sky on top of sky
				if(j > 0) {
					if(!have_prev) {
						prev = column.at(j-1);
						prev.vB = frame.rows-1-prev.vB;
						prev.vT = frame.rows-1-prev.vT;
					}

					if(sec.type == SKY && prev.type == SKY) {
						sec.vT = prev.vT;
						have_prev = true;
					} else {
						have_prev = false;
					}
				}

				// If the next segment is a sky, skip current
				if(j+1 < column.size() && sec.type == SKY && column.at(j+1).type == SKY) {
					continue;
				}
				// Don't show ground
				if(sec.type != GROUND) {
					const int x = i*column_size+width_margin;
					const int y = sec.vT;
					const int width = column_size;
					int height = sec.vB-sec.vT+1;

					cv::Mat roi = frame(cv::Rect(x, y, width, height));
				
					// Sky = blue
					int cr = 0;
					int cg = 0;
					int cb = 255;

					// Object = from green to red (far to near)
					if(sec.type == OBJECT) {
						const float dis = (max_dis_display-sec.disparity)/max_dis_display;
						float dis_f = dis;
						if(dis_f < 0.0f) {
							dis_f = 0.0f;
						}
						const float h = dis_f*0.3f;
						const float s = 1.0f;
						const float v = 1.0f;

						HSV_to_RGB(h, s, v, &cr, &cg, &cb);
					}

					cv::Mat color;
					const int top = (roi.rows < 2) ? 0 : 1;
					const int bottom = (roi.rows < 2) ? 0 : 1;
					const int left = 1;
					const int right = 1;

					color.create(roi.rows - top - bottom, roi.cols - left - right, roi.type());
					color.setTo(cv::Scalar(cb, cg, cr));

					cv::Mat color_padded;
					color_padded.create(roi.rows, roi.cols, color.type());

					copyMakeBorder(color, color_padded, top, bottom, left, right, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255) );
					const double alpha = 0.6;
					cv::addWeighted(color_padded, alpha, roi, 1.0 - alpha, 0.0, roi);

				}
			}
		}
		if(horizon_row != -1) {
			// Draw Horizon Line
			int thickness = 2;
			int lineType = 8;
			line( frame,
			cv::Point(0, horizon_row),
			cv::Point(frame.cols-1, horizon_row),
			cv::Scalar( 0, 0, 0 ),
			thickness,
			lineType );
		}

		cv::imwrite(stixelim_file, frame);
	}

	return 0;
}

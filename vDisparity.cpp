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

#include "vDisparity.h"

// OpenCV
using namespace cv;

#include <math.h>
#include <vector>


///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////////////////////////

CvDisparity::CvDisparity()
{}

CvDisparity::~CvDisparity()
{}

void CvDisparity::Initialize(const float camera_center_y, const float baseline, const float focal,
		const int max_dis) {
	// Get camera parameters
	m_cy = camera_center_y;
	m_b = baseline;
	m_focal = focal;

	// Default configuration
	m_rangeAngleX = 5;
	m_rangeAngleY = 5;
	m_HoughAccumThr = 25;
	m_binThr = 0.5f;
	m_maxPitch = 50;
	m_minPitch = -50;
	m_maxCameraHeight = 1.90f;
	m_minCameraHeight = 1.30f;

	m_maxPitch = m_maxPitch*(float)CV_PI/180.0f;
	m_minPitch = m_minPitch*(float)CV_PI/180.0f;
    m_max_dis = max_dis;

	m_showHoughTransform = true;

	m_rho = 0;
	m_theta = 0;
	m_horizonPoint = 0;
	m_pitch = 0;
	m_cameraHeight = 0;
}

void CvDisparity::Finish()
{
}

bool CvDisparity::Compute(const float *im, const int rows, const int cols) {
	bool ok = false;

	// Compute the vDisparity histogram
	ComputeHistogram(im, rows, cols);

    // Compute the Hough transform
	float rho, theta, horizonPoint, pitch, cameraHeight, slope;
	if (ComputeHough(rho, theta, horizonPoint, pitch, cameraHeight, slope))
	{
		m_rho = rho;
		m_theta = theta;
		m_horizonPoint = (int) ceil(horizonPoint);
		m_pitch = pitch;
		m_cameraHeight = cameraHeight;
		m_slope = slope;
		ok = true;
	}

	return ok;
}

void CvDisparity::ComputeHistogram(const float *im, const int rows, const int cols) {
	// Initialize and reset the vDispary histogram
	m_vDisp = Mat::zeros(rows, m_max_dis, CV_32SC1);

	// Compute the v-disparity histogram
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (im[i*cols+j] != 0.0f) {
				int col = (int) im[i*cols+j];
				m_vDisp.ptr<uint32_t>(i)[col]++;
			}
		}
	}

	// Normalize and threshold
	double max=0;
	m_vDisp.convertTo(m_vDisp,CV_32FC1);
	cv::minMaxLoc(m_vDisp, NULL, &max);
	m_vDisp=m_vDisp/max;
	cv::threshold(m_vDisp, m_vDisp, m_binThr, 255, cv::THRESH_BINARY);
	m_vDisp.convertTo(m_vDisp,CV_8UC1);

}

bool CvDisparity::ComputeHough(float& rho, float& theta, float& horizonPoint, float& pitch,
		float& cameraHeight, float& slope) {
	// Compute the Hough transform
	std::vector<Vec2f> lines;
	HoughLines(m_vDisp, lines, 1.0, CV_PI/180, m_HoughAccumThr);

	// Get the best line from hough
	size_t i;
	for (i=0; i<lines.size(); i++)
	{
		// Get rho and theta
		rho = abs(lines[i][0]);
		theta = lines[i][1];

		// Compute camera position
		ComputeCameraProperties(rho, theta, horizonPoint, pitch, cameraHeight, slope);

		if (pitch>=m_minPitch && pitch<=m_maxPitch)
			return true;
	}

	return false;
}

void CvDisparity::ComputeCameraProperties(const float rho, const float theta, float& horizonPoint,
		float& pitch, float& cameraHeight, float& slope) const {
	// Compute Horizon Line (2D)
	horizonPoint = rho/sinf(theta);

	// Compute pitch -> arctan((cy - y0Hough)/focal) It is negative because y axis is inverted
	pitch = -atanf((m_cy - horizonPoint)/(m_focal));

	// Compute the slope needed to compute the Camera height
	float last_row = (float)(m_vDisp.rows-1);
	float vDispDown = (rho-last_row*sinf(theta))/cosf(theta);
	slope = (0 - vDispDown)/(horizonPoint - last_row);

	// Compute the camera height -> baseline*cos(pitch)/slopeHough
	cameraHeight = m_b*cosf(pitch)/slope;
}

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

#ifndef _VDISPARITY_
#define _VDISPARITY_

#include <vector>
#include <opencv2/opencv.hpp>

class CStereoImage;

class CvDisparity
{
public:
	CvDisparity	();
	~CvDisparity			();

    void Initialize(const float camera_center_y, const float baseline, const float focal, const int max_dis);
	void Finish		();

	bool Compute(const float *im, const int rows, const int cols);

	float GetCameraHeight() {return m_cameraHeight;};
	float GetPitch() {return m_pitch;};
	float GetSlope() {return m_slope;};
	int GetHorizonPoint() {return m_horizonPoint;};

private:

	CvDisparity(const CvDisparity& other);
	CvDisparity& operator=(const CvDisparity& other);

	void ComputeHistogram(const float *im, const int rows, const int cols);
	void ComputeCameraProperties(const float rho, const float theta, float& horizonPoint, float& pitch,
								float& cameraHeight, float& slope) const;
	bool ComputeHough			(float& rho, float& theta, float& horizonPoint, float& pitch,
								float& cameraHeight, float& slope);


private:
	bool					m_showHoughTransform;
	int						m_rangeAngleX;			///< Angle interval to discard horizontal planes
	int						m_rangeAngleY;			///< Angle interval to discard vertical planes
	int						m_HoughAccumThr;		///< Threshold of the min number of points to form a line
	float					m_binThr;				///< Threshold to binarize vDisparity histogram
	float					m_maxPitch;				///< Angle elevation maximun of camera
	float					m_minPitch;				///< Angle elevation minimum of camera
	float					m_maxCameraHeight;		///< Height maximun of camera
	float					m_minCameraHeight;		///< Height minimun of camera
    int                     m_max_dis;

	// Member objects
	cv::Mat					m_vDisp;				///< Vertical disparity histogram
	float					m_rho;					///< Line in polar (Distance from (0,0) to the line)
	float					m_theta;				///< Line in polar (Angle of the line with x axis)

	// Auxiliar variables
	int						m_horizonPoint;			///< Horizon point of v-disparity histogram
	float					m_pitch;				///< Camera pitch
	float					m_cameraHeight;			///< Camera height
	float					m_cy;					///< Image center from stereo camera
	float					m_b;					///< Stereo camera baseline
	float					m_focal;				///< Stereo camera focal length
	float					m_slope;
};
#endif // _VDISPARITY_

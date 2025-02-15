/**
* This file is part of ORB-SLAM3
*
* Copyright (C) 2017-2020 Carlos Campos, Richard Elvira, Juan J. Gómez Rodríguez, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
* Copyright (C) 2014-2016 Raúl Mur-Artal, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
*
* ORB-SLAM3 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
* License as published by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM3 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
* the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with ORB-SLAM3.
* If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef TRACKING_H
#define TRACKING_H

#include<opencv2/core/core.hpp>
#include<opencv2/features2d/features2d.hpp>
#include <opencv2/video/tracking.hpp>

#include"Viewer.h"
#include"FrameDrawer.h"
#include"Atlas.h"
#include"LocalMapping.h"
#include"LoopClosing.h"
#include"Frame.h"
#include "ORBVocabulary.h"
#include"KeyFrameDatabase.h"
#include"ORBextractor.h"
#include "Initializer.h"
#include "MapDrawer.h"
#include "System.h"
#include "ImuTypes.h"

#include "GeometricCamera.h"

#include <mutex>
#include <unordered_set>

namespace ORB_SLAM3
{

class Viewer;
class FrameDrawer;
class Atlas;
class LocalMapping;
class LoopClosing;
class System;

class Tracking
{  

public:
    Tracking(System* pSys, ORBVocabulary* pVoc, FrameDrawer* pFrameDrawer, MapDrawer* pMapDrawer, Atlas* pAtlas,
             KeyFrameDatabase* pKFDB, const string &strSettingPath, const int sensor, const string &_nameSeq=std::string());

    ~Tracking();

    // Parse the config file
    /* !
     * @brief Camera parameter를 지정한 file에서 불러옵니다.
     * @param None
     * @return booltype
    */
    bool ParseCamParamFile(cv::FileStorage &fSettings);

    /* !
     * @brief ORB parameter를 지정한 file에서 불러옵니다.
     * @param None
     * @return booltype
    */
    bool ParseORBParamFile(cv::FileStorage &fSettings);

    /* !
     * @brief IMU parameter를 지정한 file에서 불러옵니다.
     * @param None
     * @return booltype
    */
    bool ParseIMUParamFile(cv::FileStorage &fSettings);

    /* !
     * @brief Preprocess the input and call Track(). Extract features and performs stereo matching.
     * @param None
     * @return cv matrix
    */
    cv::Mat GrabImageStereo(const cv::Mat &imRectLeft,const cv::Mat &imRectRight, const double &timestamp, string filename);

    cv::Mat GrabImageRGBD(const cv::Mat &imRGB,const cv::Mat &imD, const double &timestamp, string filename);

    cv::Mat GrabImageMonocular(const cv::Mat &im, const double &timestamp, string filename);
    // cv::Mat GrabImageImuMonocular(const cv::Mat &im, const double &timestamp);

    /* !
     * @brief IMU data를 queue형태로 저장합니다. 
     * @param None
     * @return mutex mlQueueImuData
    */
    void GrabImuData(const IMU::Point &imuMeasurement);

    /* !
    * @brief LocalMapping Class를 Pointer로 설정해주기 위한 함수
    * @param None
    * @return None
    */
    void SetLocalMapper(LocalMapping* pLocalMapper);

    /* !
    * @brief LoopClosing Class를 Pointer로 설정해주기 위한 함수
    * @param None
    * @return None
    */
    void SetLoopClosing(LoopClosing* pLoopClosing);

    /* !
    * @brief Viewer Class를 Pointer로 설정해주기 위한 함수
    * @param None
    * @return None
    */
    void SetViewer(Viewer* pViewer);

    /* !
    * @brief Bool 타입을 변수를 통해 클래스 멤버 변수 'bStepByStep'의 상태를 바꿔주는 함수
    * @param None
    * @return None
    */
    void SetStepByStep(bool bSet);

    // Load new settings
    // The focal lenght should be similar or scale prediction will fail when projecting points
    /* !
    * @brief Intrinsic calibaration 값을 대입, 초점 거리는 비슷해야 한다. 그렇지 않으면 scale 예측에 실패함.
    * @param camera_intrinsic_calibration_값에_대한_Setting_file_경로
    * @return None
    */
    void ChangeCalibration(const string &strSettingPath);

    // Use this function if you have deactivated local mapping and you only want to localize the camera.
    /* !
    * @brief Local Mapping을 비활성화하고 카메라를 이용한 locaization만 원할 때 사용
    * @param flag_for_only_Tracking
    * @return None
    */
    void InformOnlyTracking(const bool &flag);

    /* !
    * @brief IMU와 관련된 값들을 Key Frame에 update (Local mapping, Loop closing에서 쓰임) 
    * @param scale
    * @param imu_bias
    * @param current_key_frame
    * @return None
    */
    void UpdateFrameIMU(const float s, const IMU::Bias &b, KeyFrame* pCurrentKeyFrame);
    KeyFrame* GetLastKeyFrame()
    {
        return mpLastKeyFrame;
    }

    /* !
    * @brief Stereo Atlas Map을 관리하기 위한 객체 생성
    * @param None
    * @return None
    */
    void CreateMapInAtlas();
    std::mutex mMutexTracks;

    //--
    void NewDataset();
    int GetNumberDataset();
    int GetMatchesInliers();
public:

    // Tracking states
    enum eTrackingState{
        SYSTEM_NOT_READY=-1,
        NO_IMAGES_YET=0,
        NOT_INITIALIZED=1,
        OK=2,
        RECENTLY_LOST=3,
        LOST=4,
        OK_KLT=5
    };

    eTrackingState mState;
    eTrackingState mLastProcessedState;

    // Input sensor
    int mSensor;

    // Current Frame
    Frame mCurrentFrame;
    Frame mLastFrame;

    cv::Mat mImGray;

    // Initialization Variables (Monocular)
    std::vector<int> mvIniLastMatches;
    std::vector<int> mvIniMatches;
    std::vector<cv::Point2f> mvbPrevMatched;
    std::vector<cv::Point3f> mvIniP3D;
    Frame mInitialFrame;

    // Lists used to recover the full camera trajectory at the end of the execution.
    // Basically we store the reference keyframe for each frame and its relative transformation
    list<cv::Mat> mlRelativeFramePoses;
    list<KeyFrame*> mlpReferences;
    list<double> mlFrameTimes;
    list<bool> mlbLost;

    // frames with estimated pose
    int mTrackedFr;
    bool mbStep;

    // True if local mapping is deactivated and we are performing only localization
    bool mbOnlyTracking;

    /* !
    * @brief 전부 초기화 시켜주는 함수
    * @param Local Mapping이 초기화가 되어있으면 true, 초기화가 필요하면 false
    * @return None
    */
    void Reset(bool bLocMap = false);

    /* !
    * @brief Active map을 초기화 시켜주는 함수
    * @param Local Mapping이 초기화가 되어있으면 true, 초기화가 필요하면 false
    * @return None
    */
    void ResetActiveMap(bool bLocMap = false);

    float mMeanTrack;
    bool mbInitWith3KFs;
    double t0; // time-stamp of first read frame
    double t0vis; // time-stamp of first inserted keyframe
    double t0IMU; // time-stamp of IMU initialization


    /* !
    * @brief Local Map Points를 가져오는 함수
    * @param None
    * @return Map Points들이 담겨있는 Vector (pointer로 가르킴)
    */
    vector<MapPoint*> GetLocalMapMPS();

    bool mbWriteStats;

#ifdef REGISTER_TIMES
    void LocalMapStats2File();
    void TrackStats2File();
    void PrintTimeStats();

    vector<double> vdRectStereo_ms;
    vector<double> vdORBExtract_ms;
    vector<double> vdStereoMatch_ms;
    vector<double> vdIMUInteg_ms;
    vector<double> vdPosePred_ms;
    vector<double> vdLMTrack_ms;
    vector<double> vdNewKF_ms;
    vector<double> vdTrackTotal_ms;

    vector<double> vdUpdatedLM_ms;
    vector<double> vdSearchLP_ms;
    vector<double> vdPoseOpt_ms;
#endif

    vector<int> vnKeyFramesLM;  // LM은 Local Map의 줄임말
    vector<int> vnMapPointsLM;

protected:

    // Main tracking function. It is independent of the input sensor.
    void Track();

    // Map initialization for stereo and RGB-D
    /* !
    * @brief Stereo 초기화 함수 (Stereo or Stereo-IMU)
    * @param None
    * @return None
    */
    void StereoInitialization();

    // Map initialization for monocular
    void MonocularInitialization();
    void CreateNewMapPoints();      // ORB SLAM3에서는 쓰이지 않음.. 아마 ORB SLAM2의 잔재로 추정 - 관련 링크(https://github.com/UZ-SLAMLab/ORB_SLAM3/issues/363) 

    /* !
    * @brief KeyFrame 2개의 Fundamental Matrix를 구하는 함수
    * @param None
    * @return 3x3 Fundamental Matrix
    */
    cv::Mat ComputeF12(KeyFrame *&pKF1, KeyFrame *&pKF2);   Fundamental Matrix
    void CreateInitialMapMonocular();

    void CheckReplacedInLastFrame();
    bool TrackReferenceKeyFrame();
    void UpdateLastFrame();

    /* !
    * @brief Motion model을 이용하여 Tracking 하는지 판단을 위해 사용하는 함수
    * @param None
    * @return Motion model을 이용하여 Tracking하면 true, 아니면 false
    */
    bool TrackWithMotionModel();

    /* !
    * @brief IMU data의 변화량을 이용하여 현재 imu state를 예측하는 함수입니다. 
    * @param None
    * @return imu stata의 predict가 완료되면 true 실패하면 false입니다. 
    */
    bool PredictStateIMU();

    /* !
    * @brief  Tracking이 LOST 되었을 때, Relocalization 시켜주는 함수
    * @param  None
    * @return Boolean
    */
    bool Relocalization();

    /* !
    * @brief Local Map을 Update하기 위해 사용하는 함수
    * @param None
    * @return None
    */
    void UpdateLocalMap();

    /* !
    * @brief Local Map Points를 Update하기 위해 사용하는 함수
    * @param None
    * @return None
    */
    void UpdateLocalPoints();

    /* !
    * @brief Local Key Frames를 Update하기 위해 사용하는 함수 (Map point를 가장 많이 관찰하고 있는 Key Frame을 Voting을 통해 찾음)
    * @param None
    * @return None
    */
    void UpdateLocalKeyFrames();

    /* !
    * @brief Local Map을 Tracking하고 있는지 판단하기 위한 함수 (Map points의 Inlier의 갯수를 활용)
    * @param None
    * @return Local Map을 Track하고 있으면 true, 아니면 false
    */
    bool TrackLocalMap();

    bool TrackLocalMap_old();   // 쓰이지 않음.
    
    /* !
    * @brief  Local Map Points와 Current Frame의 Map Points를 매칭시켜주는 함수
    * @param  None
    * @return None
    */
    void SearchLocalPoints();

    /* !
    * @brief 새로운 KeyFrame이 필요한지 판단하기 위한 함수
    * @param None
    * @return 새로운 KeyFrame이 필요하면 True, 아니면 False
    */
    bool NeedNewKeyFrame();

    /* !
    * @brief 새로운 KeyFrame을 만드는 함수, 새로운 KeyFrame에 대한 정보들도 update
    * @param None
    * @return None
    */
    void CreateNewKeyFrame();

    /* !
    * @brief  Perform preintegration from last frame
    * @param  None
    * @return None
    */
    void PreintegrateIMU();

    // Reset IMU biases and compute frame velocity
    /* !
    * @brief  imu의 gyro bias를 재설정하는 함수입니다. 각 프레임 1번, 2번의 imu rotation, delta rotation 데이터를 통해 bias를 구하고 최신화합니다. 
    * @param  프레임이 담겨있는 vector, bias x, bias y, bias z
    * @return None
    */
    void ComputeGyroBias(const vector<Frame*> &vpFs, float &bwx,  float &bwy, float &bwz);

    /* !
    * @brief  각 프레임 1번, 2번의 imu velocity, delta position 데이터를 통해 bias를 구하고 최신화합니다. 
    * @param  프레임이 담겨있는 vector, bias acc x, bias acc y, bias acc z
    * @return None
    */
    void ComputeVelocitiesAccBias(const vector<Frame*> &vpFs, float &bax,  float &bay, float &baz);


    bool mbMapUpdated;

    // Imu preintegration from last frame
    IMU::Preintegrated *mpImuPreintegratedFromLastKF;

    // Queue of IMU measurements between frames
    std::list<IMU::Point> mlQueueImuData;

    // Vector of IMU measurements from previous to current frame (to be filled by PreintegrateIMU)
    std::vector<IMU::Point> mvImuFromLastFrame;
    std::mutex mMutexImuQueue;

    // Imu calibration parameters
    IMU::Calib *mpImuCalib;

    // Last Bias Estimation (at keyframe creation)
    IMU::Bias mLastBias;

    // In case of performing only localization, this flag is true when there are no matches to
    // points in the map. Still tracking will continue if there are enough matches with temporal points.
    // In that case we are doing visual odometry. The system will try to do relocalization to recover
    // "zero-drift" localization to the map.
    bool mbVO;

    //Other Thread Pointers
    LocalMapping* mpLocalMapper;
    LoopClosing* mpLoopClosing;

    //ORB
    ORBextractor* mpORBextractorLeft, *mpORBextractorRight;
    ORBextractor* mpIniORBextractor;

    //BoW
    ORBVocabulary* mpORBVocabulary;
    KeyFrameDatabase* mpKeyFrameDB;

    // Initalization (only for monocular)
    Initializer* mpInitializer;
    bool mbSetInit;

    //Local Map
    KeyFrame* mpReferenceKF;
    std::vector<KeyFrame*> mvpLocalKeyFrames;
    std::vector<MapPoint*> mvpLocalMapPoints;
    
    // System
    System* mpSystem;
    
    //Drawers
    Viewer* mpViewer;
    FrameDrawer* mpFrameDrawer;
    MapDrawer* mpMapDrawer;
    bool bStepByStep;

    //Atlas
    Atlas* mpAtlas;

    //Calibration matrix
    cv::Mat mK;
    cv::Mat mDistCoef;
    float mbf;

    //New KeyFrame rules (according to fps)
    int mMinFrames;
    int mMaxFrames;

    int mnFirstImuFrameId;
    int mnFramesToResetIMU;

    // Threshold close/far points
    // Points seen as close by the stereo/RGBD sensor are considered reliable
    // and inserted from just one frame. Far points requiere a match in two keyframes.
    float mThDepth;

    // For RGB-D inputs only. For some datasets (e.g. TUM) the depthmap values are scaled.
    float mDepthMapFactor;

    //Current matches in frame
    int mnMatchesInliers;

    //Last Frame, KeyFrame and Relocalisation Info
    KeyFrame* mpLastKeyFrame;
    unsigned int mnLastKeyFrameId;
    unsigned int mnLastRelocFrameId;
    double mTimeStampLost;
    double time_recently_lost;
    double time_recently_lost_visual;

    unsigned int mnFirstFrameId;
    unsigned int mnInitialFrameId;
    unsigned int mnLastInitFrameId;

    bool mbCreatedMap;


    //Motion Model
    cv::Mat mVelocity;

    //Color order (true RGB, false BGR, ignored if grayscale)
    bool mbRGB;

    list<MapPoint*> mlpTemporalPoints;

    //int nMapChangeIndex;

    int mnNumDataset;

    ofstream f_track_stats;

    ofstream f_track_times;
    double mTime_PreIntIMU;
    double mTime_PosePred;
    double mTime_LocalMapTrack;
    double mTime_NewKF_Dec;

    GeometricCamera* mpCamera, *mpCamera2;

    int initID, lastID;

    cv::Mat mTlr;

public:
    cv::Mat mImRight;
};

} //namespace ORB_SLAM

#endif // TRACKING_H

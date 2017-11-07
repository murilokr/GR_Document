#ifndef KINECT_H
#define KINECT_H

//-----------------------------------------------------------------------
//  Includes
//-----------------------------------------------------------------------
#include <XnCppWrapper.h>
#include "kmeans.hpp"

//-----------------------------------------------------------------------
//  Namespaces
//-----------------------------------------------------------------------
using namespace std;
using namespace xn;

//-----------------------------------------------------------------------
//  Defines
//-----------------------------------------------------------------------
#define SAMPLE_XML_PATH "../../../../Data/SamplesConfig.xml"
#define SAMPLE_XML_PATH_LOCAL "SamplesConfig.xml"
#define MAX_NUM_USERS 15
#define CHECK_RC(nRetVal, what)                     \
    if (nRetVal != XN_STATUS_OK)                    \
{                                   \
    printf("%s failed: %s\n", what, xnGetStatusString(nRetVal));    \
    return nRetVal;                         \
}

//-----------------------------------------------------------------------
//  Code
//-----------------------------------------------------------------------




XnBool fileExists(const char *fn)
{
    XnBool exists;
    xnOSDoesFileExist(fn, &exists);
    return exists;
}

Context g_Context;
ScriptNode g_ScriptNode;
UserGenerator g_UserGenerator;
DepthGenerator g_DepthGenerator;

XnBool g_bNeedPose;
XnChar g_strPose[20];

VideoCapture cap;

// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/){
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d New User %d\n", epochTime, nId);
    // New user found
    if (g_bNeedPose)
        g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
    else
        g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/){
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Lost user %d\n", epochTime, nId);
}

// Callback: Detected a pose
void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& /*capability*/, const XnChar* strPose, XnUserID nId, void* /*pCookie*/){
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
    g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
    g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& /*capability*/, XnUserID nId, void* /*pCookie*/){
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Calibration started for user %d\n", epochTime, nId);
}

void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& /*capability*/, XnUserID nId, XnCalibrationStatus eStatus, void* /*pCookie*/){
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    if (eStatus == XN_CALIBRATION_STATUS_OK){
        // Calibration succeeded
        printf("%d Calibration complete, start tracking user %d\n", epochTime, nId);        
        g_UserGenerator.GetSkeletonCap().StartTracking(nId);
    }
    else{
        // Calibration failed
        printf("%d Calibration failed for user %d\n", epochTime, nId);
        if(eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT){
            printf("Manual abort occured, stop attempting to calibrate!");
            return;
        }
        if (g_bNeedPose)
            g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
        else
            g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
    }
}


XnStatus Inicializar(){
    XnStatus nRetVal = XN_STATUS_OK;
    xn::EnumerationErrors errors;

    const char *fn = NULL;
    if    (fileExists(SAMPLE_XML_PATH)) fn = SAMPLE_XML_PATH;
    else if (fileExists(SAMPLE_XML_PATH_LOCAL)) fn = SAMPLE_XML_PATH_LOCAL;
    else {
        printf("Could not find '%s' nor '%s'. Aborting.\n" , SAMPLE_XML_PATH, SAMPLE_XML_PATH_LOCAL);
        return XN_STATUS_ERROR;
    }
    printf("Reading config from: '%s'\n", fn);

    nRetVal = g_Context.InitFromXmlFile(fn, g_ScriptNode, &errors);
    if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
    {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        printf("%s\n", strError);
        return (nRetVal);
    }
    else if (nRetVal != XN_STATUS_OK)
    {
        printf("Open failed: %s\n", xnGetStatusString(nRetVal));
        return (nRetVal);
    }

    nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);
    if (nRetVal != XN_STATUS_OK)
    {
        nRetVal = g_UserGenerator.Create(g_Context);
        CHECK_RC(nRetVal, "Find user generator");
    }

    nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
    if (nRetVal != XN_STATUS_OK)
    {
        nRetVal = g_DepthGenerator.Create(g_Context);
        CHECK_RC(nRetVal, "Find depth generator");
    }

    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected;
    if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
    {
        printf("Supplied user generator doesn't support skeleton\n");
        return XN_STATUS_ERROR;
    }
    nRetVal = g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
    CHECK_RC(nRetVal, "Register to user callbacks");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibrationStart);
    CHECK_RC(nRetVal, "Register to calibration start");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibrationComplete);
    CHECK_RC(nRetVal, "Register to calibration complete");

    if (g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
    {
        g_bNeedPose = TRUE;
        if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
        {
            printf("Pose required, but not supported\n");
            return XN_STATUS_ERROR;
        }
        nRetVal = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, NULL, hPoseDetected);
        CHECK_RC(nRetVal, "Register to Pose Detected");
        g_UserGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
    }

    g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

    nRetVal = g_Context.StartGeneratingAll();
    CHECK_RC(nRetVal, "StartGenerating");

    return nRetVal;
}






/**
 * createKinect
 * Função: Inicializa as funções básicas do Kinect, detectando se está conectado e se o xml esta correto.
 *  
 * Out: bool sucesso (Retorna true se o kinect está funcionando corretamente, e falso se tem algo de errado)
 */
bool createKinect(){
    if(Inicializar() != XN_STATUS_OK){
        cerr << "Erro ao Inicializar o Kinect." << endl;
        return false;
    }
    return true;
}

/**
 * createOpenCV
 * Função: Inicializa as funções do OpenCV integrado com o OpenNI do Kinect.
 *  
 * Out: bool sucesso (Retorna true se o OpenCV detectou o Kinect corretamente, e falso se tem algo de errado)
 */
bool createOpenCV(){
    VideoCapture capture(CV_CAP_OPENNI);
    capture.set(CV_CAP_OPENNI_IMAGE_GENERATOR_OUTPUT_MODE, CV_CAP_OPENNI_VGA_30HZ);
    if(!capture.isOpened()){
        cerr << "Erro ao Inicializar OpenCV com OpenNI." << endl;
        return false;
    }

    cap = capture;
    return true;
}


/**
 * getHands
 * Função: Retorna uma imagem segmentada para cada mão
 * 
 * In: Mat &leftHand (Referência à imagem da mão esquerda)
 * In: Mat &rightHand (Referência à imagem da mão direita)
 * In: Mat frame (Referência à imagem de profundidade do Kinect)
 * In: int windowSize (Tamanho da imagem para cada mão)
 * 
 * Out: Mat &leftHand
 * Out: Mat &rightHand
 */
void getHands(Mat& leftHand, Mat& rightHand, Mat frame, int windowSize){
    Mat leftHandAux, rightHandAux;
    XnUserID aUsers[MAX_NUM_USERS];
    XnUInt16 nUsers;
    XnSkeletonJointTransformation jointPosition;

    g_Context.WaitOneUpdateAll(g_UserGenerator);
    nUsers=MAX_NUM_USERS;
    g_UserGenerator.GetUsers(aUsers, nUsers);
    for(XnUInt16 i=0; i<nUsers; i++)
    {
        if(g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])==FALSE)
            continue;
                                                                   //Inverter as mãos, pois o kinect "detecta" errado (Imagem invertida).
        g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i], XN_SKEL_LEFT_HAND, jointPosition);
        XnPoint3D aProjective;        
        g_DepthGenerator.ConvertRealWorldToProjective(1,&jointPosition.position.position, &aProjective);
        int rHandX = aProjective.X;
        int rHandY = aProjective.Y;
        int rHandZ = aProjective.Z;

                                                                   //Inverter as mãos, pois o kinect "detecta" errado (Imagem invertida).
        g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i], XN_SKEL_RIGHT_HAND, jointPosition);
        g_DepthGenerator.ConvertRealWorldToProjective(1,&jointPosition.position.position, &aProjective);
        int lHandX = aProjective.X;
        int lHandY = aProjective.Y;
        int lHandZ = aProjective.Z;

        float windowHalf = (windowSize/2);
        cv::Rect roi(rHandX - windowHalf, rHandY - windowHalf, windowSize, windowSize);
        cv::Rect roiImg(0, 0, frame.cols, frame.rows);
        if( (roi.area() > 0) && ((roiImg & roi).area() == roi.area()) )
            rightHandAux = frame(roi);    
        threshold(rightHandAux, rightHand, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

        roi = Rect(lHandX - windowHalf, lHandY - windowHalf, windowSize, windowSize);
        if( (roi.area() > 0) && ((roiImg & roi).area() == roi.area()) )
            leftHandAux = frame(roi);
        threshold(leftHandAux, leftHand, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

        
        rectangle(frame, Point(lHandX-windowHalf, lHandY-windowHalf), Point(lHandX+windowHalf, lHandY+windowHalf),Scalar(255,255,255), 3);
        rectangle(frame, Point(rHandX-windowHalf, rHandY-windowHalf), Point(rHandX+windowHalf, rHandY+windowHalf),Scalar(255,255,255), 3);
    }
}


/**
 * MatText
 * Função: Imprime um texto em uma imagem
 * 
 * In: string text (O texto a ser impresso)
 * In: Mat &img (Qual imagem será alterada)
 * In: int x, y (Posição em pixels do texto na imagem)
 * 
 * Out: Mat &img
 */
void MatText(string text, Mat& img, int x, int y){
    putText(img, text, cvPoint(x,y), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);
}


/**
 * getFrame
 * Função: Retorna vetores das mãos
 * 
 * In: HC rhHC (Configuração da mão direita no frame atual)
 * In: HC lhHC (Configuração da mão esquerda no frame atual)
 * 
 * Out: Frame cFrame (Uma estrutura contendo todos os vetores calculados)
 */
Frame getFrame(HC rhHC, HC lhHC){
    XnUserID aUsers[MAX_NUM_USERS];
    XnUInt16 nUsers;
    XnSkeletonJointTransformation jointPosition;

    g_Context.WaitOneUpdateAll(g_UserGenerator);
    nUsers=MAX_NUM_USERS;
    g_UserGenerator.GetUsers(aUsers, nUsers);
    for(XnUInt16 i=0; i<nUsers; i++)
    {
        if(g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])==FALSE)
            continue;

        g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i], XN_SKEL_LEFT_HAND, jointPosition);
        XnPoint3D aProjective;        
        g_DepthGenerator.ConvertRealWorldToProjective(1,&jointPosition.position.position, &aProjective);
        int rHandX = aProjective.X;
        int rHandY = aProjective.Y;
        int rHandZ = aProjective.Z;


        g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i], XN_SKEL_RIGHT_HAND, jointPosition);
        g_DepthGenerator.ConvertRealWorldToProjective(1,&jointPosition.position.position, &aProjective);
        int lHandX = aProjective.X;
        int lHandY = aProjective.Y;
        int lHandZ = aProjective.Z;

        g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i], XN_SKEL_TORSO, jointPosition);
        g_DepthGenerator.ConvertRealWorldToProjective(1,&jointPosition.position.position, &aProjective);
        int torsoX = aProjective.X;
        int torsoY = aProjective.Y;
        int torsoZ = aProjective.Z;



        g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i], XN_SKEL_HEAD, jointPosition);
        g_DepthGenerator.ConvertRealWorldToProjective(1,&jointPosition.position.position, &aProjective);
        int headX = aProjective.X;
        int headY = aProjective.Y;

        Frame cFrame(rHandX, rHandY, rHandZ, rhHC, lHandX, lHandY, lHandZ, lhHC, torsoX, torsoY, torsoZ, headX, headY);
        return cFrame;
    }
    return Frame();
}

#endif //KINECT_H
/**
 * C++ Main - Interface para a consulta de documentos via reconhecimento de gestos
 * 
 * Copyright (c) 2017 Murilo K. Rivabem
 * All rights reserved.
 * 
*/

//-----------------------------------------------------------------------
//  Includes
//-----------------------------------------------------------------------
#include "HMM.hpp"
#include "NeuralNetwork.hpp"
#include "XLibInput.hpp"

//-----------------------------------------------------------------------
//  Defines
//-----------------------------------------------------------------------
#define MAX_BUFFER_SIZE 100
#define DEBUG_MODE 0


//-----------------------------------------------------------------------
//  Code
//-----------------------------------------------------------------------

/**
 * TrainModels
 * Função: Gera obsevações da base de dados e treina um HMM para cada gesto
 * 
 * In: KMeans *codebook (Uma referência à um objeto do tipo codebook)
 * In: HMM *advanceHMM (Modelo do gesto Avançar)
 * In: HMM *returnHMM (Modelo do gesto Retornar)
 * In: HMM *zoomInHMM (Modelo do gesto Zoom In)
 * In: HMM *zoomOutHMM (Modelo do gesto Zoom Out)
*/ 
void TrainModels(KMeans *codebook, HMM *advanceHMM, HMM *returnHMM, HMM *zoomInHMM, HMM *zoomOutHMM){
    Mat seq, subSeq; //subSeq = LOOT

    codebook->getGestureObservationsFromTrainingData("./Dataset/advanceDataTrain.txt", 40, seq, subSeq);
    cout << "Advance Observations: " << endl;
    #if DEBUG_MODE
        printMat(subSeq); cout << endl << endl;
    #endif //DEBUG_MODE
    advanceHMM->train(subSeq, 5000);

    codebook->getGestureObservationsFromTrainingData("./Dataset/returnDataTrain.txt", 40, seq, subSeq);
    cout << "Return Observations: " << endl;
    #if DEBUG_MODE
        printMat(seq); cout << endl << endl;
    #endif //DEBUG_MODE
    returnHMM->train(subSeq, 5000);

    codebook->getGestureObservationsFromTrainingData("./Dataset/zoomInDataTrain.txt", 40, seq, subSeq);
    cout << "Zoom In Observations: " << endl;
    #if DEBUG_MODE
        printMat(seq); cout << endl << endl;
    #endif //DEBUG_MODE
    zoomInHMM->train(subSeq, 5000);

    codebook->getGestureObservationsFromTrainingData("./Dataset/zoomOutDataTrain.txt", 40, seq, subSeq);
    cout << "Zoom Out Observations: " << endl;
    #if DEBUG_MODE
        printMat(seq); cout << endl << endl;
    #endif //DEBUG_MODE
    zoomOutHMM->train(subSeq, 5000);


    advanceHMM->save();
    returnHMM->save();
    zoomInHMM->save();
    zoomOutHMM->save();
}


/**
 * TestModels
 * Função: Usa o LOOT for Testing para testar o HMM
 * 
 * In: KMeans *codebook (Uma referência à um objeto do tipo codebook)
 * In: HMM *advanceHMM (Modelo do gesto Avançar)
 * In: HMM *returnHMM (Modelo do gesto Retornar)
 * In: HMM *zoomInHMM (Modelo do gesto Zoom In)
 * In: HMM *zoomOutHMM (Modelo do gesto Zoom Out)
 * In: Mat &observation (Matriz de observações)
 * 
 * Out: Mat &observation
 */
void TestModels(KMeans *codebook, HMM *advanceHMM, HMM *returnHMM, HMM *zoomInHMM, HMM *zoomOutHMM, Mat &observation){
    vector<int> *map = new vector<int>();
    for(int i = 0; i < 4; i++)
        map->push_back(0);
    
    double max = -9999;

    vector<HMM*>* models = new vector<HMM*>();
    vector<HMM*>::iterator it;
    models->push_back(advanceHMM);
    models->push_back(returnHMM);
    models->push_back(zoomInHMM);
    models->push_back(zoomOutHMM);

    int k = 0;
    int maxK = 0;
    for(int r = 0; r < observation.rows; r++){
        for(it = models->begin(); it != models->end(); ++it){
            double value = (*it)->validate(observation.row(r));
            if(value > max){
                max = value;
                maxK = k;
            }
            k++;
        }
        
        map->at(maxK) = map->at(maxK) + 1;
        k = 0;
        max = -999;
    }

    cout << "Advance: " << map->at(0) << " = " << (float)(map->at(0)*100)/observation.rows << "%" << endl;
    cout << "Return: " << map->at(1) <<  " = " << (float)(map->at(1)*100)/observation.rows << "%" << endl;
    cout << "Zoom In: " << map->at(2) << " = " << (float)(map->at(2)*100)/observation.rows << "%" << endl;
    cout << "Zoom Out: " << map->at(3) << " = " << (float)(map->at(3)*100)/observation.rows << "%" << endl;
    cout << "Total: " << observation.rows << endl << endl << endl;
}


/**
 * validateAll
 * Função: Testa a sequência para todos os modelos de HMM, e retorna o HMM mais provavel de ter gerado essa sequência.
 * 
 * In: HMM *advanceHMM (Modelo do gesto Avançar)
 * In: HMM *returnHMM (Modelo do gesto Retornar)
 * In: HMM *zoomInHMM (Modelo do gesto Zoom In)
 * In: HMM *zoomOutHMM (Modelo do gesto Zoom Out)
 * In: Mat &observation (Sequência de observações do gesto realizado)
 * 
 * Out: HMM_Name maxProb (Nome do modelo HMM que tem mais probabilidade de gerar a sequência passada)
 */
HMM_Name validateAll(HMM *advanceHMM, HMM *returnHMM, HMM *zoomInHMM, HMM *zoomOutHMM, Mat& observation){
    vector<double> *validations = new vector<double>();
    double max = -999;

    #if DEBUG_MODE
        printMat(observation);
    #endif //DEBUG_MODE

    validations->push_back(advanceHMM->validate(observation));
    validations->push_back(returnHMM->validate(observation));
    validations->push_back(zoomInHMM->validate(observation));
    validations->push_back(zoomOutHMM->validate(observation));

    int i = 0;
    int maxIndex = -1;
    for(vector<double>::iterator it = validations->begin(); it != validations->end(); ++it, ++i){
        cout << i << ": " << (*it) << endl; 
        if((*it) > max){
            max = (*it);
            maxIndex = i;
        }
    }

    //if(max < -150) //Thresholding
      //  return HMM_NoGesture;

    switch(maxIndex){
        case 0:
            return HMM_Advance;
        case 1:
            return HMM_Return;
        case 2:
            return HMM_ZoomIn;
        case 3:
            return HMM_ZoomOut;
        default:
            return HMM_NoGesture;
    }
}


void updateConfusionMatrix(Mat& conf, Mat& observation, HMM *advanceHMM, HMM *returnHMM, HMM *zoomInHMM, HMM *zoomOutHMM, HMM_Name modelName){
    int row = modelName;

    vector<int> *map = new vector<int>();
    for(int i = 0; i < 4; i++)
        map->push_back(0);
    
    double max = -9999;

    vector<HMM*>* models = new vector<HMM*>();
    vector<HMM*>::iterator it;
    models->push_back(advanceHMM);
    models->push_back(returnHMM);
    models->push_back(zoomInHMM);
    models->push_back(zoomOutHMM);

    int k = 0;
    int maxK = 0;
    for(int r = 0; r < observation.rows; r++){
        for(it = models->begin(); it != models->end(); ++it){
            double value = (*it)->validate(observation.row(r));
            if(value > max){
                max = value;
                maxK = k;
            }
            k++;
        }
        
        map->at(maxK) = map->at(maxK) + 1;
        k = 0;
        max = -999;
    }

    for(int c = 0; c < conf.cols; c++)
        conf.at<int>(row, c) = conf.at<int>(row,c) + map->at(c);
    
}


void drawConfusionMatrix(KMeans *Codebook, HMM *advanceModel, HMM *returnModel, HMM *zoomInModel, HMM *zoomOutModel){
    Mat seq, subSeq;
    Mat conf = cv::Mat(4,4, CV_32SC1);
    conf = Mat::zeros(4, 4, CV_32SC1);

    Codebook->getGestureObservationsFromTrainingData("./Dataset/advanceDataTrain.txt", 40, seq, subSeq);
    updateConfusionMatrix(conf, seq, advanceModel, returnModel, zoomInModel, zoomOutModel, HMM_Advance);

    Codebook->getGestureObservationsFromTrainingData("./Dataset/returnDataTrain.txt", 40, seq, subSeq);
    updateConfusionMatrix(conf, seq, advanceModel, returnModel, zoomInModel, zoomOutModel, HMM_Return);

    Codebook->getGestureObservationsFromTrainingData("./Dataset/zoomInDataTrain.txt", 40, seq, subSeq);
    updateConfusionMatrix(conf, seq, advanceModel, returnModel, zoomInModel, zoomOutModel, HMM_ZoomIn);

    Codebook->getGestureObservationsFromTrainingData("./Dataset/zoomOutDataTrain.txt", 40, seq, subSeq);
    updateConfusionMatrix(conf, seq, advanceModel, returnModel, zoomInModel, zoomOutModel, HMM_ZoomOut);

    HMM_Name temp;
    cout << "\t\t" << HMM_ToString(HMM_Advance) << "\t\t" << HMM_ToString(HMM_Return) << "\t\t" << HMM_ToString(HMM_ZoomIn) << "\t\t" << HMM_ToString(HMM_ZoomOut) << endl;
    for(int r = 0; r < conf.rows; r++){
        cout << HMM_ToString(intToHMM(r)) << "\t\t";
        for(int c = 0; c < conf.cols; c++){
            cout << conf.at<int>(r,c) << "\t\t";
        }
        cout << endl << endl;
    }
}


int main(int argc, char* argv[]){
    setlocale(LC_ALL, "C");

    
    string filename = "./Dataset/codebook16.txt";
    fstream data(filename.c_str(), ios::in);

    KMeans *Codebook = new KMeans(data);
    data.close();    


    //srand(time(NULL));
    //int max = 15;
    //int min = 5;
    int stateNumber = 9;//rand()%(max-min + 1) + min;
    //cout << "Number of States: " << stateNumber << endl;
    //cout << "Number of Symbols: " << Codebook->getClusterNumber() << endl;

    HMM *advanceModel, *returnModel, *zoomInModel, *zoomOutModel;
    advanceModel = new HMM("advance.hmm", Codebook->getClusterNumber(), stateNumber);
    returnModel = new HMM("return.hmm", Codebook->getClusterNumber(), stateNumber);
    zoomInModel = new HMM("zoomIn.hmm", Codebook->getClusterNumber(), stateNumber);
    zoomOutModel = new HMM("zoomOut.hmm", Codebook->getClusterNumber(), stateNumber);

    HandConfiguration *leftHandNN, *rightHandNN;
    leftHandNN = new HandConfiguration("./Data/lefthand.net");
    rightHandNN = new HandConfiguration("./Data/righthand.net");
    if(!leftHandNN->loadNet()){
        cerr << "Error loading ./Data/lefthand.net" << endl;
        return -1;
    }
    if(!rightHandNN->loadNet()){
        cerr << "Error loading ./Data/righthand.net" << endl;
        return -1;
    }


    HC lhHC, rhHC;

    if(!advanceModel->isAlreadyModeled() || !returnModel->isAlreadyModeled() || !zoomInModel->isAlreadyModeled() || !zoomOutModel->isAlreadyModeled()){
        cout << "Training HMM Models..." << endl;
        TrainModels(Codebook, advanceModel, returnModel, zoomInModel, zoomOutModel);
        return 0;
    }

    if(argc == 2){
        Mat seq, subSeq;
        Codebook->getGestureObservationsFromTrainingData(argv[1], 40, seq, subSeq);
        TestModels(Codebook, advanceModel, returnModel, zoomInModel, zoomOutModel, seq);
        return 0;
    }

    
    //drawConfusionMatrix(Codebook, advanceModel, returnModel, zoomInModel, zoomOutModel);

    if(!createKinect())
        return -1;
    if(!createOpenCV())
        return -1;
    
    
    Mat frame, leftHandFrame, rightHandFrame;
    bool endit = false;
    bool isRecording = false;


    int windowSize = 75;
    double scaleDownFactor = 0.75;
    namedWindow("Depth Image", 1);
    namedWindow("Left Hand", 1);
    namedWindow("Right Hand", 1);


    bool recordFrames = false;
    int maxFrames = 40;
    vector<Frame>* frameBuffer = new vector<Frame>(maxFrames);
    Frame currentFrame;

    float torsoHeight = -99999;
    float rY, lY;

    Mat hmmObservation;
    HMM_Name gesture;

    cout << "Para visualizar um documento, focalize uma janela pdf e realize os gestos." << endl;

    while(!endit){
        while (xnOSWasKeyboardHit()){

            char c = xnOSReadCharFromInput();
            if(c == 27) endit = !endit;
        }
        gesture = HMM_NoGesture;

        cap.grab();
        cap.retrieve(frame, CV_CAP_OPENNI_DISPARITY_MAP);
        getHands(leftHandFrame, rightHandFrame, frame, windowSize);
        lhHC = HC_notDefined;
        rhHC = HC_notDefined;

        if(!leftHandFrame.empty()){
            resize(leftHandFrame, leftHandFrame, Size(), 0.75, 0.75);
            lhHC = leftHandNN->evaluate(leftHandFrame);
            if(lhHC == HC_error){
                cerr << "Error in left hand image. (Possibly lost user reference)" << endl;
                continue;
            }
            string lInfo = "Left Hand: " + HC_ToString(lhHC);
            MatText(lInfo, frame, 10, 10);
        }
        if(!rightHandFrame.empty()){
            resize(rightHandFrame, rightHandFrame, Size(), 0.75, 0.75);
            rhHC = rightHandNN->evaluate(rightHandFrame);
            if(rhHC == HC_error){
                cerr << "Error in right hand image. (Possibly lost user reference)" << endl;
                continue;
            }
            string rInfo = "Right Hand: " + HC_ToString(rhHC);
            MatText(rInfo, frame, 10, 40);
        }



        currentFrame = getFrame(rhHC, lhHC);
        if(currentFrame.conf != 1)
            continue;
        torsoHeight = currentFrame.getTorsoY();
        rY = currentFrame.getRightY();
        lY = currentFrame.getLeftY();

        if(rY < torsoHeight || lY < torsoHeight){
            if(!recordFrames){
                cout << "Começar a gravar" << endl;
                recordFrames = true;
                frameBuffer->clear();
                frameBuffer->push_back(currentFrame);
            }
        }else
            if(recordFrames){
                cout << "Parar" << endl;
                recordFrames = false;
            }


        if(recordFrames){
            if(frameBuffer->size() < maxFrames){
                frameBuffer->push_back(currentFrame);
            }else{
                Codebook->realTimeObservations(frameBuffer, frameBuffer->size(), hmmObservation);
                gesture = validateAll(advanceModel, returnModel, zoomInModel, zoomOutModel, hmmObservation);
                cout << "HMM Detected: " << HMM_ToString(gesture) << endl;
                frameBuffer->clear();
                recordFrames = false;
            }
        }



        if(gesture != HMM_NoGesture){
            Action ac;
            switch(gesture){
                case HMM_Advance:
                    ac = KeyRight;
                    break;
                case HMM_Return:
                    ac = KeyLeft;
                    break;
                case HMM_ZoomIn:
                    ac = KeyZoomIn;
                    break;
                case HMM_ZoomOut:
                    ac = KeyZoomOut;
                    break;
            }
            SendInput(ac);
        }
        

        cv::imshow("Depth Image", frame);
        if(!leftHandFrame.empty())
            cv::imshow("Left Hand", leftHandFrame);
        if(!rightHandFrame.empty())
            cv::imshow("Right Hand", rightHandFrame);
        char esc = cv::waitKey(33);
        if (esc == 27) break;
    }
    
    /*
    Mat observations, subObs;
    Codebook->getGestureObservationsFromTrainingData(argv[1], 40, observations, subObs);
    //printMat(subObs);
    
    double validation;
    for(int i = 0; i < observations.rows; i++){
        cout << "Validation " << i << endl;

        validation = advanceModel->validate(observations.row(i));
        cout << "Advance HMM: " << validation << endl;


        validation = returnModel->validate(observations.row(i));
        cout << "Return HMM: " << validation << endl;

        validation = zoomInModel->validate(observations.row(i));
        cout << "Zoom In HMM: " << validation << endl;

        validation = zoomOutModel->validate(observations.row(i));
        cout << "Zoom Out HMM: " << validation << endl;

        cout << endl << endl;
    }
    */
    g_ScriptNode.Release();
    g_UserGenerator.Release();
    g_DepthGenerator.Release();
    g_Context.Release();
    return 0;
}
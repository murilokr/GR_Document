#include "Kinect.hpp"
#include "CvHMM.h"
#include <sstream>

enum HMM_Name{
    HMM_Error = -2,
    HMM_NoGesture = -1,
    HMM_Advance = 0,
    HMM_Return = 1,
    HMM_ZoomIn = 2,
    HMM_ZoomOut = 3
};

HMM_Name intToHMM(int i){
    switch(i){
        case 0:
            return HMM_Advance;
            break;
        case 1:
            return HMM_Return;
            break;
        case 2:
            return HMM_ZoomIn;
            break;
        case 3:
            return HMM_ZoomOut;
            break;
        default:
            return HMM_NoGesture;
            break;
    }
    return HMM_NoGesture;
}

/**
 * HMM_ToString
 * Função: Converte um enum do tipo HMM_Name para string
 * 
 * In: HMM_Name hmm (Enumerador do gesto)
 * Out: string retVal (Enumerador em string)
 */
string HMM_ToString(HMM_Name hmm){
    string retVal = "";
    switch(hmm){
        case 0:
            retVal = "Advance Gesture";
            break;
        case 1:
            retVal = "Return Gesture";
            break;
        case 2:
            retVal = "Zoom-In Gesture";
            break;
        case 3:
            retVal = "Zoom-Out Gesture";
            break;
        default:
            retVal = "Undefined Gesture";
            break;
    }
    return retVal;
}


class HMM{
private:
    Mat TRANS, EMIS, INIT; //Model
    string modelType;
    bool alreadyModeled;

    /**
     * CreateRandomHMM
     * Função: Cria um modelo HMM aleatório
     * 
     * In: int codebookSize (O tamanho do codebook)
     * In: int stateNumber (O numero de estados)
     */
    void CreateRandomHMM(int codebookSize, int stateNumber){
        srand(time(NULL));

        int dimension = stateNumber * stateNumber;
        double *TRANSdata = new double[dimension];
        double *EMISdata = new double[stateNumber * codebookSize];
        double *INITdata = new double[stateNumber];

        

        double randomValue;
        for(int i = 0; i < dimension; i++){
            randomValue = ((double) rand() / (RAND_MAX));
            TRANSdata[i] = randomValue;
        }

        int a;
        for(int i = 0; i < (stateNumber * codebookSize); i++){
            a = rand()%2;
            if(a == 0)
                EMISdata[i] = 0.0;
            else
                EMISdata[i] = stateNumber/codebookSize;
        }

        double max = 1.0;
        for(int i = 0; i < stateNumber; i++){
            if(max < 0.05){
                max = 0.0;
                INITdata[i] = 0.0;
            }
            else{
                do{
                    randomValue = ((double) rand() / (RAND_MAX));
                }while(randomValue > max);
                max -= randomValue;
                INITdata[i] = randomValue;
            }
        }

        TRANS = cv::Mat(stateNumber, stateNumber, CV_64F, TRANSdata).clone();
        EMIS = cv::Mat(stateNumber, codebookSize, CV_64F, EMISdata).clone();
        INIT = cv::Mat(1, stateNumber, CV_64F, INITdata).clone();
    }

public:

    /**
     * HMM
     * Função: Construtor da classe HMM
     * 
     * In: string type (O tipo do modelo HMM)
     * In: int codebookSize (O tamanho do codebook)
     * 
     * Out: HMM *hmm (Um objeto HMM criado)
     */
    HMM(string type, int codebookSize, int stateNumber) : alreadyModeled(false){
        modelType = type;
        if(!load())
            CreateRandomHMM(codebookSize, stateNumber);
        else
            alreadyModeled = true;
    }

    /**
     * getTransitionMatrix | getEmissionMatrix | getInitialMatrix
     * Função: Retorna a matriz de TRANSITION, EMISSION e INITIAL
     * 
     * In: Mat &data (Uma Mat para copiar a matriz TRANS)
     * Out: Mat &data
     */
    void getTransitionMatrix(Mat& data){data = TRANS;}
    void getEmissionMatrix(Mat& data){data = EMIS;}
    void getInitialMatrix(Mat& data){data = INIT;}


    /**
     * load
     * Função: Carrega o arquivo .hmm para um novo objeto
     * 
     * Out: bool sucesso (Retorna verdadeiro se foi possível carregar o arquivo, e falso se o arquivo não existe ou foi corrompido)
     */
    bool load(){
        string filename = "./Data/" + modelType;
        fstream file(filename.c_str(), ios::in);
        if(!file.is_open())
            return false;

        int N, M;
        double value;
        file >> N >> M;
        TRANS = cv::Mat(N,M,CV_64F);
        for(int r = 0; r < TRANS.rows; r++){
            for(int c = 0; c < TRANS.cols; c++){
                file >> value;
                TRANS.at<double>(r,c) = value;
            }
        }

        file >> N >> M;
        EMIS = cv::Mat(N,M,CV_64F);
        for(int r = 0; r < EMIS.rows; r++){
            for(int c = 0; c < EMIS.cols; c++){
                file >> value;
                EMIS.at<double>(r,c) = value;
            }
        }

        file >> N >> M;
        INIT = cv::Mat(N,M,CV_64F);
        for(int r = 0; r < INIT.rows; r++){
            for(int c = 0; c < INIT.cols; c++){
                file >> value;
                INIT.at<double>(r,c) = value;
            }
        }

        return true;
    }


    /**
     * save
     * Função: Salva um modelo HMM para um arquivo .hmm
     * 
     * Out: bool sucesso (Retorna true se foi possível criar o arquivo, e falso se deu algo errado)
     */
    bool save(){
        string filename = "./Data/" + modelType;
        fstream file(filename.c_str(), ios::out | ios::trunc);
        if(!file.is_open())
            return false;
        
        int N, M;
        N = TRANS.rows;
        M = TRANS.cols;

        file << N << "\t" << M;
        file << endl;
        for(int r=0; r<TRANS.rows; r++)
            for(int c=0; c<TRANS.cols; c++)
                file << TRANS.at<double>(r,c) << "\t";
        file << endl;

        N = EMIS.rows;
        M = EMIS.cols;
        file << N << "\t" << M;
        file << endl;
        for(int r=0; r<EMIS.rows; r++)
            for(int c=0; c<EMIS.cols; c++)
                file << EMIS.at<double>(r,c) << "\t";
        file << endl;

        N = INIT.rows;
        M = INIT.cols;
        file << N << "\t" << M;
        file << endl;
        for(int r=0; r<INIT.rows; r++)
            for(int c=0; c<INIT.cols; c++)
                file << INIT.at<double>(r,c) << "\t";
        file << endl;

        return true;
    }

    /**
     * train
     * Função: Treina um modelo HMM
     * 
     * In: Mat &seq (Uma sequência de observações)
     * In: int max_iter (Número de iterações para o treinamento)
     */
    void train(Mat &seq, int max_iter){
        CvHMM cvhmm;
        cvhmm.train(seq, max_iter, TRANS, EMIS, INIT);

        //cout << "TRANS: " << endl;
        //printMat(TRANS); cout << endl << endl;
        //cout << "EMIS: " << endl;
        //printMat(EMIS); cout << endl << endl;
        //cout << "INIT: " << endl;
        //printMat(INIT); cout << endl << endl;
    }

    /**
     * validate
     * Função: Executa o modelo HMM usando uma sequência de observações
     * 
     * In: Mat &seq (A matriz de observações)
     * 
     * Out: double logpseq (A probabilidade em log que esse HMM gera a sequência passada)
     */
    double validate(const Mat &seq){
        CvHMM cvhmm;
        cv::Mat STATES,FORWARD,BACKWARD;
        double logpseq;
        cvhmm.decode(seq, TRANS, EMIS, INIT, logpseq, STATES, FORWARD, BACKWARD);

        return logpseq;
    }

    void print(){
        CvHMM cvhmm;
        cvhmm.printModel(TRANS,EMIS,INIT);
    }

    bool isAlreadyModeled(){
        return alreadyModeled;
    }


    
};
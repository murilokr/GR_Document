#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <locale.h>
#include <time.h>

using namespace cv;
using namespace std;


enum HC{
    HC_error = -2,
    HC_notDefined = -1,
    HC_advance = 0,
    HC_return = 1,
    HC_zoomIn = 2,
    HC_zoomOut = 3
};

typedef struct Frame{
    public:
        int rightHandX, rightHandY, rightHandZ, handConfigurationRight;
        int leftHandX, leftHandY, leftHandZ, handConfigurationLeft;
        int torsoX, torsoY, torsoZ;
        int headX, headY;
        int htRatioX, htRatioY;
        float rightVectorX, rightVectorY, rightVectorZ;
        float leftVectorX, leftVectorY, leftVectorZ;
        int conf;

    Frame(){
        conf = 0;
    }
    /**
     * Frame
     * Função: Gera um frame com base nas posições de joints no mundo
     * 
     * In: int rhX (Posição X da mão direita)
     * In: int rhY (Posição Y da mão direita)
     * In: int rhZ (Posição Z da mão direita)
     * In: int hc1 (Configuração da mão direita)
     * In: int lhX (Posição X da mão esquerda)
     * In: int lhY (Posição Y da mão esquerda)
     * In: int lhZ (Posição Z da mão esquerda)
     * In: int hc2 (Configuração da mão esquerda)
     * In: int tX (Posição X do torso)
     * In: int tY (Posição Y do torso)
     * In: int tZ (Posição Z do torso)
     * In: int hX (Posição X da cabeça)
     * In: int hY (Posição Y da cabeça)
    */ 
    Frame(int rhX, int rhY, int rhZ, int hc1, int lhX, int lhY, int lhZ, int hc2, int tX, int tY, int tZ, int hX, int hY){
        rightHandX = rhX;
        rightHandY = rhY;
        rightHandZ = rhZ;
        handConfigurationRight = hc1;

        leftHandX = lhX;
        leftHandY = lhY;
        leftHandZ = lhZ;
        handConfigurationLeft = hc2;


        torsoX = tX;
        torsoY = tY;
        torsoZ = tZ;

        headX = hX;
        headY = hY;

        htRatioX = sqrt(pow(torsoX - headX, 2));
        htRatioY = sqrt(pow(torsoY - headY, 2));
        

        rightVectorX = rightHandX - torsoX;
        rightVectorY = rightHandY - torsoY;
        rightVectorZ = rightHandZ - torsoZ;

        leftVectorX = leftHandX - torsoX;
        leftVectorY = leftHandY - torsoY;
        leftVectorZ = leftHandZ - torsoZ;

        int htRatio = sqrt(pow(htRatioX, 2) + pow(htRatioY, 2));

        rightVectorX /= htRatio;
        rightVectorY /= htRatio;


        leftVectorX /= htRatio;
        leftVectorY /= htRatio;

        conf = 1;
    }


    float getZLength(){return sqrt(rightVectorZ*rightVectorZ);}
    float getLeftY(){return leftHandY;}
    float getRightY(){return rightHandY;}
    float getTorsoY(){return torsoY;}
    
}Frame;


struct Centroids{
    float rightVectorX, rightVectorY, rightVectorZ, rightHandConfiguration;
    float leftVectorX, leftVectorY, leftVectorZ, leftHandConfiguration;


    void print(){
        cout << rightVectorX << "\t" << rightVectorY << "\t" << rightVectorZ << "\t" << rightHandConfiguration;
        cout << leftVectorX << "\t" << leftVectorY << "\t" << leftVectorZ << "\t" << leftHandConfiguration;
    }
}typedef Centroids;

void printMat(Mat& data){
    for (int i=0;i<data.rows;i++)
    {
        std::cout << i << ": ";
        for (int j=0;j<data.cols;j++)
            std::cout << data.at<int>(i,j) << "|";
        std::cout << "\n";
    }
}

class KMeans{

    private:
        vector<Centroids> *codebook;

        /**
         * ReadFromFile
         * Função: Cria um codebook usando centroides salvos em um arquivo
         * 
         * In: fstream &file (Referência ao arquivo)
         */
        void ReadFromFile(fstream& file){
            if(!file.is_open())
                return;
            

            float rVx, rVy, rVz, rHc;
            float lVx, lVy, lVz, lHc;

            while(!file.eof()){
                file >> rVx >> rVy >> rVz >> rHc;
                file >> lVx >> lVy >> lVz >> lHc;

                Centroids c = {rVx, rVy, rVz, rHc, lVx, lVy, lVz, lHc};
                codebook->push_back(c);
            }
        }

    public:
        KMeans(){
            codebook = new vector<Centroids>();
        };
        
        KMeans(vector<Centroids>* c){
            codebook = c;
        }

        KMeans(fstream& file){
            codebook = new vector<Centroids>();
            ReadFromFile(file);
        }

        void PrintCodebook(){
            int i = 0;
            for(vector<Centroids>::iterator it = codebook->begin(); it != codebook->end(); ++it){
                cout << i << ": ";
                (*it).print();
                cout << endl;
                i++;
            }
        }

        int getClusterNumber(){
            return codebook->size();
        }

        vector<Centroids>* returnCentroids(){
            return codebook;
        }


        bool isEmpty(){
            return codebook->empty();
        }

        /**
         * GetNearestCluster
         * Função: Calcula a distância do centroide passado para todos que existem no Codebook
         * 
         * In: Centroids coordinates (Coordenadas recebidas)
         * 
         * Out: int clstNumb (Número do cluster em que a coordenada pertence)
         */
        int GetNearestCluster(Centroids coordinates){
            if(isEmpty())
                return -1;

            int clstNumb = 0, currClst = 0;
            float minDst = 10000;

            for(vector<Centroids>::iterator cluster = codebook->begin(); cluster != codebook->end(); ++cluster){
                float dXr = (*cluster).rightVectorX - coordinates.rightVectorX;                        
                dXr *= dXr;
                float dYr = (*cluster).rightVectorY - coordinates.rightVectorY;
                dYr *= dYr;
                float dZr = (*cluster).rightVectorZ - coordinates.rightVectorZ;
                dZr *= dZr;
                float dHCr = (*cluster).rightHandConfiguration - coordinates.rightHandConfiguration;
                dHCr *= dHCr;

                float dXl = (*cluster).leftVectorX - coordinates.leftVectorX;
                dXl *= dXl;
                float dYl = (*cluster).leftVectorY - coordinates.leftVectorY;
                dYl *= dYl;
                float dZl = (*cluster).leftVectorZ - coordinates.leftVectorZ;
                dZl *= dZl;
                float dHCl = (*cluster).leftHandConfiguration - coordinates.leftHandConfiguration;
                dHCl *= dHCl;

                float d = sqrt(dXr + dYr + dZr + dHCr + dXl + dYl + dZl + dHCl);
                if(d < minDst){
                    minDst = d;
                    clstNumb = currClst;
                }
                currClst++;
            }

            return clstNumb;
        }

        vector<int>* returnObservations(vector<Centroids>* clusters){

            vector<int>* observations = new vector<int>();

            for(vector<Centroids>::iterator coord = clusters->begin(); coord != clusters->end(); ++coord){
                int code = GetNearestCluster(*coord);
                observations->push_back(code);
            }

            return observations;
        }



        /**
         * lootStrategy
         * Função: Gera subsequências dada uma sequência original
         * 
         * In: Mat &sequence (Sequência original)
         * In: Mat &subSequence (Subsequência)
         * 
         * Out: Mat &sequence
         * Out: Mat &subSequence
         */
        void lootStrategy(Mat& sequence, Mat& subSequence){
            cout << "Sequence Rows: " << sequence.rows << endl;
            vector<int> observation;
            vector<int> loot;

            int rowSize = sequence.cols * sequence.rows;
            int columnSize = sequence.cols - 1;
            subSequence = cv::Mat(rowSize, columnSize, CV_32SC1);

            int k = 0;
            int p = 0;
            for(int r = 0; r<sequence.rows; r++){
                sequence.row(r).copyTo(observation);
                
                for(int i = 0; i<observation.size(); i++){
                    loot = observation;
                    loot.erase(loot.begin()+i);

                    for(int c=0; c<subSequence.cols; c++){
                        subSequence.at<int>(k,c) = loot[p];
                        p++;
                    }
                    k++;
                    p = 0;
                }
            }
        }

        //Return a matrix with NxM dimensions, where:
        //N = the number of times a gesture was made / sequence size (unknown)
        //M = the size of the gesture (known)
        /**
         * getGestureObservationsFromTrainingData
         * Função: Retorna uma matriz onde as linhas são as sequencias de movimentos de um arquivo, e as colunas são as observações desse movimento
         * 
         * In: string filename (Nome do arquivo para obter as observacoes)
         * In: int gestureSize (Número de frames do gesto)
         * In: Mat &observationsMat (Matriz de observações)
         * 
         * Out: Mat &observationsMat (Matriz de observações)
        */ 
        void getGestureObservationsFromTrainingData(string filename, int gestureSize, cv::Mat &observationsMat, cv::Mat &subSeq){
            fstream file(filename.c_str(), ios::in);
            if(!file.is_open())
                return;

            vector<Centroids>* coordinates = new vector<Centroids>();
            
            float rVx, rVy, rVz, rHc;
            float lVx, lVy, lVz, lHc;

            while(!file.eof()){
                file >> rVx >> rVy >> rVz >> rHc;
                file >> lVx >> lVy >> lVz >> lHc;

                Centroids c = {rVx, rVy, rVz, rHc, lVx, lVy, lVz, lHc};
                coordinates->push_back(c);
            }
            file.close();

            vector<int> *observationsVector = returnObservations(coordinates);
            int nmbSeq = (int)(observationsVector->size()/gestureSize);
            observationsMat = cv::Mat(nmbSeq, gestureSize, CV_32SC1);
            
            vector<int>::iterator it = observationsVector->begin();
            for(int r = 0; r < observationsMat.rows; r++){
                for(int c = 0; c < observationsMat.cols; c++){
                    observationsMat.at<int>(r,c) = (*it);
                    ++it;
                }
            }
    
            lootStrategy(observationsMat, subSeq);
        }



        /**
         * realTimeObservations
         * Função: Calcula a sequência de observações dado um vetor de coordenadas
         * 
         * In: vector<Frame>* framesBuffer (Buffer onde está armazenadas as coordenadas)
         * In: int gestureSize (Número de frames do gesto)
         * In: Mat &observationsMat (Matriz de observações)
         * 
         * Out: Mat &observationsMat (Matriz de observações)
        */ 
        void realTimeObservations(vector<Frame>* framesBuffer, int gestureSize, cv::Mat &observationsMat){
            vector<Centroids>* coordinates = new vector<Centroids>();
            
            float rVx, rVy, rVz, rHc;
            float lVx, lVy, lVz, lHc;

            for(vector<Frame>::iterator it = framesBuffer->begin(); it != framesBuffer->end(); ++it){
                rVx = (*it).rightVectorX;
                rVy = (*it).rightVectorY;
                rVz = (*it).rightVectorZ;
                rHc = (*it).handConfigurationRight;
                lVx = (*it).leftVectorX;
                lVy = (*it).leftVectorY;
                lVz = (*it).leftVectorZ;
                lHc = (*it).handConfigurationLeft;

                Centroids c = {rVx, rVy, rVz, rHc, lVx, lVy, lVz, lHc};
                coordinates->push_back(c);
            }

            vector<int> *observationsVector = returnObservations(coordinates);
            int nmbSeq = (int)(observationsVector->size()/gestureSize);
            observationsMat = cv::Mat(nmbSeq, gestureSize, CV_32SC1);
            
            vector<int>::iterator it = observationsVector->begin();
            for(int r = 0; r < observationsMat.rows; r++){
                for(int c = 0; c < observationsMat.cols; c++){
                    observationsMat.at<int>(r,c) = (*it);
                    ++it;
                }
            }
        }
};
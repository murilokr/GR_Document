#ifndef NEURALNETWORK_HPP
#define NEURALNETWORK_HPP

#include "Kinect.hpp"
#include <floatfann.h>

/**
 * HC_ToString
 * Função: Converte um enum do tipo HC para string
 * 
 * In: HC handConfig (Enumerador da rede)
 * Out: string retVal (Enumerador em string)
 */
string HC_ToString(HC handConfig){
    string retVal = "";
    switch(handConfig){
        case 0:
            retVal = "Advance";
            break;
        case 1:
            retVal = "Return";
            break;
        case 2:
            retVal = "Zoom-In";
            break;
        case 3:
            retVal = "Zoom-Out";
            break;
        default:
            retVal = "Undefined";
            break;
    }
    return retVal;
}

class HandConfiguration{

    private:
        string str_netPath;
        struct fann *ann;

        /**
         * returnOutput
         * Função: Retorna o output que tem a maior probabilidade
         * 
         * In: fann_type *calc_out (Output da Rede Neural)
         * In: int size (Tamanho da camada output)
         * 
         * Out: HC retVal (Enumerador da configuração de mão)
         */
        HC returnOutput(fann_type *calc_out, int size){
            fann_type max = -999;
            int enumIt = 0;

            for(int i = 0; i < size; i++)
                if(calc_out[i] > max){
                    max = calc_out[i];
                    enumIt = i;
                }
            

            HC retVal;
            switch(enumIt){
                case 0:
                    retVal = HC_advance;
                    break;
                case 1:
                    retVal = HC_return;
                    break;
                case 2:
                    retVal = HC_zoomIn;
                    break;
                case 3:
                    retVal = HC_zoomOut;
                    break;
                default:
                    retVal = HC_notDefined;
                    break;
            }
            if(max < 0.97)//0.85) //Thresholding
                retVal = HC_notDefined;

            return retVal;
        }
        
    public:
        HandConfiguration(string netPath) : str_netPath(netPath){}
        ~HandConfiguration(){
            fann_destroy(ann);
        }

        /**
         * loadNet
         * Função: Carrega um arquivo .net
         * 
         * Out: bool sucesso (Retorna true se a rede foi carregada com sucesso, e falso se algo deu errado)
         */
        bool loadNet(){
            ann = fann_create_from_file(str_netPath.c_str());

            if(ann != NULL)
                return true;
            return false;
        }

        /**
         * evaluate
         * Função: Executa a rede neural em uma imagem
         * 
         * In: Mat inpMat (Imagem a ser reconhecida pela rede)
         * 
         * Out: HC retVal (Configuração de mão reconhecida)
         */
        HC evaluate(Mat inpMat){
            int dim = inpMat.cols * inpMat.rows;
            if(dim != ann->num_input)
                return HC_error;

            fann_type *calc_out;
            fann_type input[dim];

            int k = 0;
            for(int r = 0; r < inpMat.rows; r++){
                for(int c = 0; c < inpMat.cols; c++){
                    float bin = (float)inpMat.at<uchar>(r,c);
                    if(bin > 128)
                        bin = 1.0;
                    else
                        bin = 0.0;
                    input[k] = bin;
                    k++;
                }
            }

            calc_out = fann_run(ann, input);

            /*cout << "< ";
            for(int i=0; i < 4; i++)
                cout << calc_out[i] << " ";
            cout << ">" << endl;*/

            return returnOutput(calc_out, 4);
        }

};

#endif //NEURALNETWORK_HPP
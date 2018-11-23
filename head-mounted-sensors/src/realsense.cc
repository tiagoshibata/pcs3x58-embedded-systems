#include <opencv2/opencv.hpp>
#include <librealsense/rs.hpp>

uint16_t* filterData(uint16_t* data, int width, int height, uint16_t range) {
    static uint16_t filtered[9999999];
    int i;
    uint16_t minEsquerda = 65535;
    uint16_t minDireita = 65535;
    int meiota = 314;
     for (i=0;i<(width*height);i++){
        //linha no quadrante esquerdo
        if (!((data[i]/meiota)%2)) {
            if (data[i] > 0 && data[i] < minEsquerda) {
                minEsquerda = data[i];
                //linha no quadrante direito
            }
        } else {
            if (data[i] > 0 && data[i] < minDireita) {
                minDireita = data[i];
            }
        }
    }
    uint16_t maxDireita = minDireita + range;
    uint16_t maxEsquerda = minEsquerda + range;
    for (i=0;i<(width*height);i++) {
        if (!((data[i]/meiota)%2)) {
        // Na esquerda
            if (data[i] >= maxEsquerda) {
                filtered[i] = 0;
            } else if (data[i] <= minEsquerda) {
                filtered[i] = 0;
            } else {
                filtered[i] = 65535;
            }
        } else {
        // Na direita
            if (data[i] >= maxDireita) {
                filtered[i] = 0;
            } else if (data[i] <= minDireita) {
                filtered[i] = 0;
            } else {
                filtered[i] = 65535;
            }
        }
    }
    return filtered;
 }

 int normalize(int min, int max, double left, double right) {
    int angle = (int) ((left - right) * (max - (min)) / (30 - (-30)));

    // Clipping
    if (angle < min)
        return min;
    if (angle > max)
        return max;
    return angle;
}

int getAxis(cv::Mat data, int filter) {
    int i, j;
    int left = 0;
    int right = 0;
    int nLeft = 0;
    int nRight = 0;
    float meiota = data.cols/2;
     for(i = 0; i < (data.rows - filter); i++) {
        if (i > filter) {
            for(j = 0; j < (data.cols - filter); j++) {
                if (j > filter) {
                    if (data.at<int>(i,j) != 0) {
                        if (j <= meiota) { // está na esquerda
                            nLeft = nLeft + 1;
                            left = left + i;
                        } else { // está na direita
                            nRight = nRight + 1;;
                            right = right + i;
                        }
                    }
                }
            }
        }
    }
    double mLeft = left/(nLeft+1);
    double mRight = right/(nRight+1);
    return normalize(-128,127,mLeft,mRight);
 }

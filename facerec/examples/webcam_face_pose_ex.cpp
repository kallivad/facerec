// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

    This example program shows how to find frontal human faces in an image and
    estimate their pose.  The pose takes the form of 68 landmarks.  These are
    points on the face such as the corners of the mouth, along the eyebrows, on
    the eyes, and so forth.  
    

    This example is essentially just a version of the face_landmark_detection_ex.cpp
    example modified to use OpenCV's VideoCapture object to read from a camera instead 
    of files.


    Finally, note that the face detector is fastest when compiled with at least
    SSE2 instructions enabled.  So if you are using a PC with an Intel or AMD
    chip then you should enable at least SSE2 instructions.  If you are using
    cmake to compile this program you can enable them by using one of the
    following commands when you create the build project:
        cmake path_to_dlib_root/examples -DUSE_SSE2_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_SSE4_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_AVX_INSTRUCTIONS=ON
    This will set the appropriate compiler options for GCC, clang, Visual
    Studio, or the Intel compiler.  If you are using another compiler then you
    need to consult your compiler's manual to determine how to enable these
    instructions.  Note that AVX is the fastest but requires a CPU from at least
    2011.  SSE4 is the next fastest and is supported by most current machines.  
*/

#include <dlib/opencv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
//#include <opencv2/core/types.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/serialize.h>

#include <dlib/dnn.h>
#include <mysql.h>
#include <json/json.h>
#include <chrono>

//#include "resnet/dlib_resnet.h"
#include "npd/npddetect.h"
#include "npd/npdmodel.h"
#include "npd/npdtable.h"

//#include "SimplePocoHandler.h"
//#include "AMQPcpp.h"
#include <AMQPcpp.h>

using namespace dlib;
using namespace std;

template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N, BN, 1, tag1<SUBNET>>>;

template <template <int, template<typename>class, int, typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2, 2, 2, 2, skip1<tag2<block<N, BN, 2, tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block = BN<con<N, 3, 3, 1, 1, relu<BN<con<N, 3, 3, stride, stride, SUBNET>>>>>;

template <int N, typename SUBNET> using ares = relu<residual<block, N, affine, SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block, N, affine, SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256, SUBNET>;
template <typename SUBNET> using alevel1 = ares<256, ares<256, ares_down<256, SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128, ares<128, ares_down<128, SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64, ares<64, ares<64, ares_down<64, SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32, ares<32, ares<32, SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128, avg_pool_everything<
	alevel0<
	alevel1<
	alevel2<
	alevel3<
	alevel4<
	max_pool<3, 3, 2, 2, relu<affine<con<32, 7, 7, 2, 2,
	input_rgb_image_sized<150>
	>>>>>>>>>>>>;
	


namespace npd{

    npdmodel::npdmodel()
    {
        init();
    }

    npdmodel::npdmodel(const char* modelpath)
    {
        init();
        load(modelpath);
    }

    npdmodel::~npdmodel()
    {
        release();
    }

    void npdmodel::init()
    {
        m_objSize = 0;
        m_numStages = 0;
        m_numScales = 0;
        m_numLeafNodes = 0;
        m_numBranchNodes = 0;
        m_scaleFactor = 0.0;

        m_cutpoint = NULL;
        m_winSize = NULL;
        m_treeRoot = NULL;
        m_leftChild = NULL;
        m_rightChild = NULL;
        m_pixelx = NULL;
        m_pixely = NULL;
        m_stageThreshold = NULL;
        m_fit = NULL;

        // ******************** TEMP *********************
        //prepare();
    }

    void npdmodel::release()
    {
        int i;
        if(m_cutpoint != NULL)
        {
            for(i = 0; i < 2; i++)
            {
                free(m_cutpoint[i]);
                m_cutpoint[i] = NULL;
            }
            free(m_cutpoint);
        }
        m_cutpoint = NULL;

        if(m_winSize != NULL)
        {
            free(m_winSize);
        }
        m_winSize = NULL;

        if(m_treeRoot != NULL)
        {
            free(m_treeRoot);
        }
        m_treeRoot = NULL;

        if(m_leftChild != NULL)
        {
            free(m_leftChild);
        }
        m_leftChild = NULL;

        if(m_rightChild != NULL)
        {
            free(m_rightChild);
        }
        m_rightChild = NULL;

        if(m_pixelx != NULL)
        {
            for(i = 0; i < m_numScales; i++)
            {
                free(m_pixelx[i]);
                m_pixelx[i] = NULL;
            }
            free(m_pixelx);
        }
        m_pixelx = NULL;

        if(m_pixely != NULL)
        {
            for(i = 0; i < m_numScales; i++)
            {
                free(m_pixely[i]);
                m_pixely[i] = NULL;
            }
            free(m_pixely);
        }
        m_pixely = NULL;
        
        if(m_stageThreshold != NULL)
        {
            free(m_stageThreshold);
        }
        m_stageThreshold = NULL;

        if(m_fit != NULL)
        {
            free(m_fit);
        }
        m_fit = NULL;

        m_objSize = 0;
        m_numStages = 0;
        m_numScales = 0;
        m_numLeafNodes = 0;
        m_numBranchNodes = 0;
        m_scaleFactor = 0.0;

    }

    void npdmodel::prepare(int os, int nst, int nb, int nl, 
            float sf, int nsa)
    {
//        m_objSize = 20;
//        m_numStages = 72;
//        m_numBranchNodes = 1018;
//        m_numLeafNodes = 1090;
//        m_scaleFactor = 1.2;
//        m_numScales = 30;

        m_objSize = os;
        m_numStages = nst;
        m_numBranchNodes = nb;
        m_numLeafNodes = nl;
        m_scaleFactor = sf;
        m_numScales = nsa;

        int i;


        m_stageThreshold = (float*)malloc(m_numStages * sizeof(float));
        m_treeRoot = (int*)malloc(m_numStages * sizeof(int));

        m_pixelx = (int**)malloc(m_numScales * sizeof(int*));
        m_pixely = (int**)malloc(m_numScales * sizeof(int*));
        for(i = 0; i < m_numScales; i++)
        {
            m_pixelx[i] = (int*)malloc(m_numBranchNodes * sizeof(int));
            m_pixely[i] = (int*)malloc(m_numBranchNodes * sizeof(int));
        }

        m_cutpoint = (unsigned char**)malloc(2 * sizeof(unsigned char*));
        for(i = 0; i < 2; i++)
        {
            m_cutpoint[i] = (unsigned char*)malloc(m_numBranchNodes * sizeof(unsigned char));
        }
        
        m_leftChild = (int*)malloc(m_numBranchNodes * sizeof(int));
        m_rightChild = (int*)malloc(m_numBranchNodes * sizeof(int));

        m_fit = (float*)malloc(m_numLeafNodes * sizeof(float));

        m_winSize = (int*)malloc(m_numScales * sizeof(int));
    }

    void npdmodel::load(const char* modelpath)
    {
	    printf("Load models:%s ...\n", modelpath);

	    int n = 0;
	    int i;
	    int os, nst, nb, nl, nsa; 
	    float sf;
	    FILE* fp = fopen(modelpath, "rb");
	    size_t rs;

	    rs = fread(&os, sizeof(int), 1, fp);
	    rs = fread(&nst, sizeof(int), 1, fp);
	    rs = fread(&nb, sizeof(int), 1, fp);
	    rs = fread(&nl, sizeof(int), 1, fp);
	    rs = fread(&sf, sizeof(float), 1, fp);
	    rs = fread(&nsa, sizeof(int), 1, fp);

	    // Malloc space.
	    prepare(os, nst, nb, nl, sf, nsa);

	    rs = fread(m_stageThreshold, sizeof(float), m_numStages, fp);
	    n += rs;
	    rs = fread(m_treeRoot, sizeof(int), m_numStages, fp);
	    n += rs;
	    for(i = 0; i < m_numScales; i++)
	    {
		    rs = fread(m_pixelx[i], sizeof(int), m_numBranchNodes, fp);
		    n += rs;
	    }
	    for(i = 0; i < m_numScales; i++)
	    {
		    rs = fread(m_pixely[i], sizeof(int), m_numBranchNodes, fp);
		    n += rs;
	    }
	    for(i = 0; i < 2; i++)
	    {
		    rs = fread(m_cutpoint[i], sizeof(unsigned char), m_numBranchNodes, fp);
		    n += rs;
	    }
	    rs = fread(m_leftChild, sizeof(int), m_numBranchNodes, fp);
	    n += rs;
	    rs = fread(m_rightChild, sizeof(int), m_numBranchNodes, fp);
	    n += rs;
	    rs = fread(m_fit, sizeof(float), m_numLeafNodes, fp);
	    n += rs;
	    rs = fread(m_winSize, sizeof(int), m_numScales, fp);
	    n += rs;

	    fclose(fp);
        printf("Models loaded(load %d bytes)!!!\n", n);
    }
}

	
	
string NumberToString(int val)
    {
        stringstream ss;
         ss << val;
         return ss.str();
    }

namespace npd{

    npddetect::npddetect()
    {
        init(m_model.m_objSize, 5000);
    }

    npddetect::npddetect(int minFace, int maxFace)
    {
        init(minFace, maxFace);
    }

    npddetect::~npddetect()
    {
        release();
    }

    //npddetect::npddetect()
    //{
     //   init();

        //load(modelpath);
    //}

    void npddetect::init(int minFace, int maxFace)
    {
        m_minFace = minFace;
        m_maxFace = maxFace;
        m_overlappingThreshold = 0.5;
        m_maxScanNum = 0;
        m_maxDetectNum = 0;

        m_Tpredicate    = NULL;
        m_Troot         = NULL;
        m_Tlogweight    = NULL;
        m_Tparent       = NULL;
        m_Trank         = NULL;
        mallocsacnspace(500);

        m_Tneighbors    = NULL;
        m_Tweight       = NULL;
        m_Txs           = NULL;
        m_Tys           = NULL;
        m_Tss           = NULL;
        mallocdetectspace(40);
    }

    void npddetect::mallocsacnspace(int s)
    {
        if(s > m_maxScanNum)
            freesacnspace();

        // Malloc.
        m_Tpredicate = (char*)malloc(sizeof(char) * s * s);
        if(m_Tpredicate == NULL)
            return;

        m_Troot = (int*)malloc(sizeof(int) * s);
        if(m_Troot == NULL)
            return;

        m_Tlogweight = (float*)malloc(sizeof(float) * s);
        if(m_Tlogweight == NULL)
            return;

        m_Tparent = (int*)malloc(sizeof(int) * s);
        if(m_Tparent == NULL)
            return;

        m_Trank = (int*)malloc(sizeof(int) * s);
        if(m_Trank == NULL)
            return;

        m_maxScanNum = s;
    }

    void npddetect::freesacnspace()
    {
        if(m_Tpredicate != NULL)
            free(m_Tpredicate);
        m_Tpredicate = NULL;

        if(m_Troot != NULL)
            free(m_Troot);
        m_Troot = NULL;

        if(m_Tlogweight != NULL)
            free(m_Tlogweight);
        m_Tlogweight = NULL;

        if(m_Tparent != NULL)
            free(m_Tparent);
        m_Tparent = NULL;

        if(m_Trank != NULL)
            free(m_Trank);
        m_Trank = NULL;
    }

    void npddetect::mallocdetectspace(int n)
    {
        if(n > m_maxDetectNum)
            freedetectspace();

        // Malloc.
        m_Tneighbors = (int*)malloc(sizeof(int) * n);
        if(m_Tneighbors == NULL)
            return;

        m_Tweight = (float*)malloc(sizeof(float) * n);
        if(m_Tweight == NULL)
            return;

        m_Txs = (float*)malloc(sizeof(float) * n);
        if(m_Txs == NULL)
            return;

        m_Tys = (float*)malloc(sizeof(float) * n);
        if(m_Tys == NULL)
            return;

        m_Tss = (float*)malloc(sizeof(float) * n);
        if(m_Tss == NULL)
            return;

        m_maxDetectNum = n;
    }

    void npddetect::freedetectspace()
    {
        if(m_Tneighbors != NULL)
            free(m_Tneighbors);
        m_Tneighbors = NULL;

        if(m_Tweight != NULL)
            free(m_Tweight);
        m_Tweight = NULL;

        if(m_Txs != NULL)
            free(m_Txs);
        m_Txs = NULL;

        if(m_Tys != NULL)
            free(m_Tys);
        m_Tys = NULL;

        if(m_Tss != NULL)
            free(m_Tss);
        m_Tss = NULL;
    }

    void npddetect::release()
    {
        freesacnspace();
        freedetectspace();
    }

    void npddetect::load(const char* modelpath)
    {
        m_model.load(modelpath);
    }

    int npddetect::detect(const unsigned char* I, int width, int height)
    {
        // Clear former data.
        reset();

        // Scan with model.
        m_numScan = scan(I, width, height);
        if(m_numScan > m_maxScanNum)
        {
            if(2*m_maxScanNum > m_numScan)
                mallocsacnspace(m_maxScanNum * 2);
            else
                mallocsacnspace(m_numScan + m_maxScanNum);
        }

        // Merge rect.
        m_numDetect = filter();

        return m_numDetect;
    }

    int npddetect::prescandetect(const unsigned char* I, int width, int height, double stepR, double thresR)
    {
        // Clear former data.
        reset();

        //double time = double(cvGetTickCount());
        //double frequency = double(cvGetTickFrequency());
        // Scan with model.
        m_numScan = gridScan(I, width, height, stepR, thresR);
        //1printf("gridScan time: %fms", (cvGetTickCount() - time)/frequency );

        if(m_numScan > m_maxScanNum)
        {
            if(2*m_maxScanNum > m_numScan)
                mallocsacnspace(m_maxScanNum * 2);
            else
                mallocsacnspace(m_numScan + m_maxScanNum);
        }

        // Merge rect.
        //time = cvGetTickCount();
        m_numDetect = filter();
        //printf("filter time: %fms", (cvGetTickCount() - time)/frequency );

        return m_numDetect;
    }

    void npddetect::reset()
    {
        m_xs.clear();
        m_ys.clear();
        m_sizes.clear();
        m_scores.clear();
        m_Xs.clear();
        m_Ys.clear();
        m_Ss.clear();
        m_Scores.clear();
        m_numScan = 0;
    }

    int npddetect::gridScan(const unsigned char* I, int width, int height, double stepR, double thresR)
    {
        //int minFace = m_model.m_objSize;
        int minFace = max(m_minFace, m_model.m_objSize);
        int maxFace = min(m_maxFace, min(height, width));

        if(min(height, width) < minFace)
            return 0;

        //printf("w:%d h:%d min:%d max:%d numScale:%d\n",
        //        width, height, minFace, maxFace, m_model.m_numScales);

        cv::Mat scoreMat(height, width, CV_32FC1, cv::Scalar::all(0));

        int k;
        //double tcnt = 0.0f;
        for(k = 0; k < m_model.m_numScales; k++) // process each scale
        {
            //printf("at scale: %d\n", m_model.m_winSize[k]);

            if(m_model.m_winSize[k] < minFace) continue;
            else if(m_model.m_winSize[k] > maxFace) break;

            // determine the step of the sliding subwindow
            int winStep = (int) floor(m_model.m_winSize[k] * stepR);
	    if(m_model.m_winSize[k] > 40) winStep = (int) floor(m_model.m_winSize[k] * 0.5 * stepR);
            //double t = (double)cvGetTickCount();
            // calculate the offset values of each pixel in a subwindow
            // pre-determined offset of pixels in a subwindow
            std::vector<int> offset(m_model.m_winSize[k] * m_model.m_winSize[k]);
            int p1 = 0, p2 = 0, gap = width;

            for(int j=0; j < m_model.m_winSize[k]; j++) // column coordinate
            {
                p2 = j;
                for(int i = 0; i < m_model.m_winSize[k]; i++) // row coordinate
                {
                    offset[p1++] = p2;
                    p2 += gap;
                }
            }
            //t = ((double)cvGetTickCount() - t) / ((double)cvGetTickFrequency()*1000.) ;
            //tcnt += t;

            int colMax = width - m_model.m_winSize[k] + 1;
            int rowMax = height - m_model.m_winSize[k] + 1;

            double minVal = 99999, maxVal = -99999;

            // process each subwindow
            for(int r = 0; r < rowMax; r += winStep) // slide in row
            {
                const unsigned char *pPixel = I + r * width;;
                for(int c = 0; c < colMax; c += winStep, pPixel += winStep) // slide in column

                {
                    int treeIndex = 0;
                    float _score = 0;
                    int s;

                    // test each tree classifier
                    for(s = 0; s < m_model.m_numStages; s++)
                    {
                        int node = m_model.m_treeRoot[treeIndex];

                        // test the current tree classifier
                        while(node > -1) // branch node
                        {
                            unsigned char p1 = pPixel[
                                offset[m_model.m_pixelx[k][node]]];
                            unsigned char p2 = pPixel[
                                offset[m_model.m_pixely[k][node]]];
                            unsigned char fea = npdTable[p2][p1];
                            //printf("w[0][0]=%d\n", pPixel[0]);
                            //printf("r = %d, c = %d, k = %d, node = %d, fea = %d, cutpoint = (%d, %d), p1off = %d, p2off = %d, p1x = %d, p2x = %d, p1 = %d, p2 = %d, winsize = %d\n",
                            //        r, c, k, node, int(fea), int(m_model.m_cutpoint[0][node]), int(m_model.m_cutpoint[1][node]),
                            //        offset[m_model.m_pixelx[k][node]], offset[m_model.m_pixely[k][node]],
                            //        m_model.m_pixelx[k][node], m_model.m_pixely[k][node], p1, p2, m_model.m_winSize[k]);

                            if(fea < m_model.m_cutpoint[0][node]
                                    || fea > m_model.m_cutpoint[1][node])
                                node = m_model.m_leftChild[node];
                            else
                                node = m_model.m_rightChild[node];
                        }

                        // leaf node
                        node = - node - 1;
                        _score = _score + m_model.m_fit[node];
                        treeIndex++;

                        //printf("stage = %d, score = %f\n", s, _score);
                        if(_score < m_model.m_stageThreshold[s])
                            break; // negative samples
                    }

                    scoreMat.at<float>(r, c) = _score;
                    if(_score < minVal)  minVal = _score ;
                    if(_score > maxVal)  maxVal = _score ;

                } // Cols.
            } // Row.


            floodScoreMat(scoreMat, rowMax, colMax, winStep);
            double GridThreshold = minVal +( maxVal - minVal) * thresR;
            //printf("GridThreshold: %f\n", GridThreshold);

            // determine the step of the sliding subwindow
            winStep = (int) floor(m_model.m_winSize[k] * 0.1);
            if(m_model.m_winSize[k] > 40) winStep = (int) floor(m_model.m_winSize[k] * 0.05);

            //double t = (double)cvGetTickCount();
            // calculate the offset values of each pixel in a subwindow
            // pre-determined offset of pixels in a subwindow

            //t = ((double)cvGetTickCount() - t) / ((double)cvGetTickFrequency()*1000.) ;
            //tcnt += t;

            // process each subwindow
            for(int r = 0; r < rowMax; r += winStep) // slide in row
            {
                const unsigned char *pPixel = I + r * width;;
                for(int c = 0; c < colMax; c += winStep, pPixel += winStep) // slide in column
                {
                    int treeIndex = 0;
                    float _score = 0;
                    int s = 0;

                    if(scoreMat.at<float>(r,c) < GridThreshold)
                        continue;
                    // test each tree classifier
                    for(s = 0; s < m_model.m_numStages; s++)
                    {
                        int node = m_model.m_treeRoot[treeIndex];

                        // test the current tree classifier
                        while(node > -1) // branch node
                        {
                            unsigned char p1 = pPixel[
                                offset[m_model.m_pixelx[k][node]]];
                            unsigned char p2 = pPixel[
                                offset[m_model.m_pixely[k][node]]];
                            unsigned char fea = npdTable[p2][p1];
                            //printf("w[0][0]=%d\n", pPixel[0]);
                            //printf("r = %d, c = %d, k = %d, node = %d, fea = %d, cutpoint = (%d, %d), p1off = %d, p2off = %d, p1x = %d, p2x = %d, p1 = %d, p2 = %d, winsize = %d\n",
                            //        r, c, k, node, int(fea), int(m_model.m_cutpoint[0][node]), int(m_model.m_cutpoint[1][node]),
                            //        offset[m_model.m_pixelx[k][node]], offset[m_model.m_pixely[k][node]],
                            //        m_model.m_pixelx[k][node], m_model.m_pixely[k][node], p1, p2, m_model.m_winSize[k]);

                            if(fea < m_model.m_cutpoint[0][node]
                                    || fea > m_model.m_cutpoint[1][node])
                                node = m_model.m_leftChild[node];
                            else
                                node = m_model.m_rightChild[node];
                        }

                        // leaf node
                        node = - node - 1;
                        _score = _score + m_model.m_fit[node];
                        treeIndex++;

                        //printf("stage = %d, score = %f\n", s, _score);
                        if(_score < m_model.m_stageThreshold[s])
                            break; // negative samples
                    }

                    if(s == m_model.m_numStages) // a face detected
                    {
                        m_ys.push_back(r + 1);
                        m_xs.push_back(c + 1);
                        m_sizes.push_back(m_model.m_winSize[k]);
                        m_scores.push_back(_score);
                    }
                } // Cols.
            } // Row.

        } // Scale.

        //for(int i = 0; i < scoreMats.size(); i++)
        //{
        //    double minVal, maxVal;
        //    cv::minMaxLoc(scoreMats[i], &minVal, &maxVal);
        //    scoreMats[i] -= minVal;
        //    cv::Mat B;
        //    scoreMats[i].convertTo(B, CV_8UC1, 255.0/(maxVal - minVal));
        //    cv::namedWindow("result" + NumberToString(i));
        //    cv::imshow("result" + NumberToString(i), B);
        //}

        //cv::waitKey(-1);

        return m_ys.size();

    }



    int npddetect::floodScoreMat(cv::Mat& mat, int rowMax, int colMax, int winStep)
    {
        int rows = mat.rows;
        int cols = mat.cols;

        int yGridNum = rowMax / winStep;
        int xGridNum = colMax / winStep;


         //for cols
        for(int c = 0; c < colMax; c+=winStep)
        {
             for(int g = 0; g < yGridNum; g++)
             {
                 int begin = g*winStep;
                 int end = begin+winStep;
                 float beginVal = mat.at<float>(begin, c);
                 float endVal = mat.at<float>(end, c);
                 //if(beginVal == 0 || endVal == 0)
                 //{
                     //printf("Warning: grid scores have 0 val ! begin: %d end: %d col: %d beginVal: %f endVal: %f\n", begin, end, c, beginVal, endVal );
                 //}

                 for(int r = begin + 1; r < end; r ++)
                 {
                      mat.at<float>(r, c) = ((r - begin)*endVal + (end - r)*beginVal)/winStep;
                 }
             }

             int begin = yGridNum *winStep;
             int end = rows;
             int step = rows - begin;
             float beginVal = mat.at<float>(begin, c);
             float endVal = mat.at<float>(0, c);
             for(int r = begin + 1; r < end ;r++)
             {
                  mat.at<float>(r, c) = ((r - begin)*endVal + (end - r)*beginVal)/step;
             }

        }

        //for rows
        for(int r = 0; r < rows ; r++)
        {

             float* rowHead = mat.ptr<float>(r);
             for(int g = 0; g < xGridNum; g++)
             {
                int begin = g*winStep;
                int end = begin + winStep;
                float beginVal = rowHead[begin];
                float endVal = rowHead[end];
                //if(beginVal == 0 || endVal == 0)
                //{
                    //printf("Warning: grid scores have 0 val ! begin: %d end: %d row: %d beginVal: %f endVal: %f\n", begin, end, r, beginVal, endVal );
                //}

                for(int c = begin + 1; c < end; c++)
                {
                     rowHead[c] = ((c - begin) * endVal + (end - c) * beginVal)/winStep;
                }
             }

             int begin = xGridNum *winStep;
             int end = cols;
             int step = end - begin;
             float beginVal = rowHead[begin];
             float endVal = rowHead[0];
             for(int c = begin + 1; c < end ;c++)
             {
                 rowHead[c] = ((c - begin) * endVal + (end - c) * beginVal)/step;
             }
        }

        return 0;
    }

    int npddetect::scan(const unsigned char* I, int width, int height)
    {
        //int minFace = m_model.m_objSize;
        int minFace = max(m_minFace, m_model.m_objSize);
        int maxFace = min(m_maxFace, min(height, width));

        if(min(height, width) < minFace)
            return 0;

        //printf("w:%d h:%d min:%d max:%d numScale:%d\n",
        //        width, height, minFace, maxFace, m_model.m_numScales);

        int k;
        //double tcnt = 0.0f;
        for(k = 0; k < m_model.m_numScales; k++) // process each scale
        {
            if(m_model.m_winSize[k] < minFace) continue;
            else if(m_model.m_winSize[k] > maxFace) break;

            // determine the step of the sliding subwindow
            int winStep = (int) floor(m_model.m_winSize[k] * 0.1);
            if(m_model.m_winSize[k] > 40) winStep = (int) floor(m_model.m_winSize[k] * 0.05);

            //double t = (double)cvGetTickCount();
            // calculate the offset values of each pixel in a subwindow
            // pre-determined offset of pixels in a subwindow
            std::vector<int> offset(m_model.m_winSize[k] * m_model.m_winSize[k]);
            int p1 = 0, p2 = 0, gap = width;

            for(int j=0; j < m_model.m_winSize[k]; j++) // column coordinate
            {
                p2 = j;
                for(int i = 0; i < m_model.m_winSize[k]; i++) // row coordinate
                {
                    offset[p1++] = p2;
                    p2 += gap;
                }
            }
            //t = ((double)cvGetTickCount() - t) / ((double)cvGetTickFrequency()*1000.) ;
            //tcnt += t;

            int colMax = width - m_model.m_winSize[k] + 1;
            int rowMax = height - m_model.m_winSize[k] + 1;

            // process each subwindow
            for(int r = 0; r < rowMax; r += winStep) // slide in row
            {
                const unsigned char *pPixel = I + r * width;;
                for(int c = 0; c < colMax; c += winStep, pPixel += winStep) // slide in column

                {
                    int treeIndex = 0;
                    float _score = 0;
                    int s;

                    // test each tree classifier
                    for(s = 0; s < m_model.m_numStages; s++)
                    {
                        int node = m_model.m_treeRoot[treeIndex];

                        // test the current tree classifier
                        while(node > -1) // branch node
                        {
                            unsigned char p1 = pPixel[
                                offset[m_model.m_pixelx[k][node]]];
                            unsigned char p2 = pPixel[
                                offset[m_model.m_pixely[k][node]]];
                            unsigned char fea = npdTable[p2][p1];
                            //printf("w[0][0]=%d\n", pPixel[0]);
                            //printf("r = %d, c = %d, k = %d, node = %d, fea = %d, cutpoint = (%d, %d), p1off = %d, p2off = %d, p1x = %d, p2x = %d, p1 = %d, p2 = %d, winsize = %d\n",
                            //        r, c, k, node, int(fea), int(m_model.m_cutpoint[0][node]), int(m_model.m_cutpoint[1][node]),
                            //        offset[m_model.m_pixelx[k][node]], offset[m_model.m_pixely[k][node]],
                            //        m_model.m_pixelx[k][node], m_model.m_pixely[k][node], p1, p2, m_model.m_winSize[k]);

                            if(fea < m_model.m_cutpoint[0][node]
                                    || fea > m_model.m_cutpoint[1][node])
                                node = m_model.m_leftChild[node];
                            else
                                node = m_model.m_rightChild[node];
                        }

                        // leaf node
                        node = - node - 1;
                        _score = _score + m_model.m_fit[node];
                        treeIndex++;

                        //printf("stage = %d, score = %f\n", s, _score);
                        if(_score < m_model.m_stageThreshold[s])
                            break; // negative samples
                    }

                    if(s == m_model.m_numStages) // a face detected
                    {
                        m_ys.push_back(r + 1);
                        m_xs.push_back(c + 1);
                        m_sizes.push_back(m_model.m_winSize[k]);
                        m_scores.push_back(_score);
                    }
                } // Cols.
            } // Row.
        } // Scale.

        //printf("Userd %lf(ms) for scan...\n", 0);

        return (int) m_ys.size();
    }

    float logistic(float s)
    {
        return log(1 + exp(double(s)));
    }


    int npddetect::filter()
    {
        if(m_numScan <= 0)
            return 0;

        int i, j, ni, nj;
        float h, w, s, si, sj;
        //char* predicate = (char*)malloc(
        //        sizeof(char) * m_numScan * m_numScan);

        //printf("maxScan:%d numScan:%d mPredict:%d\n",
        //        m_maxScanNum, m_numScan, m_Tpredicate);

        memset(m_Tpredicate, 0,
                sizeof(char) * m_numScan * m_numScan);

        // mark nearby detections
        for(i = 0; i < m_numScan; i++)
        {
            ni = i * m_numScan;
            for(j = 0; j < m_numScan; j++)
            {
                nj = j * m_numScan;
                h = min(m_ys[i] + m_sizes[i],
                        m_ys[j] + m_sizes[j]) -
                    max(m_ys[i], m_ys[j]);
                w = min(m_xs[i] + m_sizes[i],
                        m_xs[j] + m_sizes[j]) -
                    max(m_xs[i], m_xs[j]);
                s = max(double(h),0.0) * max(double(w),0.0);
                si = m_sizes[i]*m_sizes[i];
                sj = m_sizes[j]*m_sizes[j];

                // 1. Overlap 50%
                if((s / (si + sj - s)) >=
                        m_overlappingThreshold)
                {
                    m_Tpredicate[ni + j] = 1;
                    m_Tpredicate[nj + i] = 1;
                }

                // 2. Overlap 80% of small one.
                //if(s / si >= 0.8 || s / sj >= 0.8)
                //{
                //    m_Tpredicate[ni + j] = 1;
                //    m_Tpredicate[nj + i] = 1;
                //}
            }
        }

        //int* root = (int*)malloc(sizeof(int) * m_numScan);
        for(i = 0; i < m_numScan; i++)
            m_Troot[i] = -1;
        int n = partition(m_Tpredicate, m_Troot);
        if(n > m_maxDetectNum)
            mallocdetectspace(n + 40);

        //float* logweight = (float*)malloc(sizeof(float) * m_numScan);
        for(i = 0; i < m_numScan; i++)
        {
            m_Tlogweight[i] = logistic(m_scores[i]);
        }

        //int* neighbors = (int*)malloc(sizeof(int) * n);
        //float* weight = (float*)malloc(sizeof(float) * n);
        //float* xs = (float*)malloc(sizeof(float) * n);
        //float* ys = (float*)malloc(sizeof(float) * n);
        //float* ss = (float*)malloc(sizeof(float) * n);
        memset(m_Tweight, 0, sizeof(float) * n);
        memset(m_Tneighbors, 0, sizeof(int) * n);
        memset(m_Txs, 0, sizeof(float) * n);
        memset(m_Tys, 0, sizeof(float) * n);
        memset(m_Tss, 0, sizeof(float) * n);
        for(i = 0; i < m_numScan; i++)
        {
            m_Tweight[m_Troot[i]] += m_Tlogweight[i];
            m_Tneighbors[m_Troot[i]] += 1;
        }

        for(i = 0; i < m_numScan; i++)
        {
            if(m_Tweight[m_Troot[i]] != 0)
                m_Tlogweight[i] /= m_Tweight[m_Troot[i]];
            else
                m_Tlogweight[i] = 1 / m_Tneighbors[m_Troot[i]];
            m_Txs[m_Troot[i]] += m_xs[i] * m_Tlogweight[i];
            m_Tys[m_Troot[i]] += m_ys[i] * m_Tlogweight[i];
            m_Tss[m_Troot[i]] += m_sizes[i] * m_Tlogweight[i];
        }

        //printf("Detect %d faces:\n", n);
        for(i = 0; i < n; i++)
        {
            //printf("%fx%fx%fx%f %f\n", xs[i], ys[i], ss[i], ss[i], weight[i]);
            m_Xs.push_back(int(m_Txs[i]));
            m_Ys.push_back(int(m_Tys[i]));
            m_Ss.push_back(int(m_Tss[i]));
            m_Scores.push_back((m_Tweight[i]));
        }

//        free(predicate);
//        free(root);
//        free(logweight);
//        free(neighbors);
//        free(weight);
//        free(xs);
//        free(ys);
//        free(ss);

        return n;
    }

    int findRoot(int* parent, int i)
    {
        if(parent[i] != i)
            return findRoot(parent, parent[i]);
        else
            return i;
    }

    int npddetect::partition(char* predicate, int* root)
    {
        //int* parent = (int*)malloc(sizeof(int) * m_numScan);
        //int* rank = (int*)malloc(sizeof(int) * m_numScan);
        int i, j, ni;
        int root_i, root_j;
        for(i = 0; i < m_numScan; i++)
            m_Tparent[i] = i;
        m_Trank[i] = 0;

        ni = 0;
        for(i = 0; i < m_numScan; i++)
        {
            for(j = 0; j < m_numScan; j++, ni++)
            {
                if(predicate[ni] == 0)
                    continue;

                root_i = findRoot(m_Tparent, i);
                root_j = findRoot(m_Tparent, j);

                if(root_i != root_j)
                {
                    if(m_Trank[i] > m_Trank[j])
                        m_Tparent[root_j] = root_i;
                    else if(m_Trank[i] < m_Trank[j])
                        m_Tparent[root_i] = root_j;
                    else
                    {
                        m_Tparent[root_j] = root_i;
                        m_Trank[root_i] ++;
                    }
                }
            }
        }

        int n = 0;
        for(i = 0; i < m_numScan; i++)
        {
            if(m_Tparent[i] == i)
            {
                if(root[i] == -1)
                    root[i] = n++;
                continue;
            }

            root_i = findRoot(m_Tparent, i);
            if(root[root_i] == -1)
                root[root_i] = n++;
            root[i] = root[root_i];
        }

        //free(rank);
        //free(parent);

        return n;
    }

}
	

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(-1);
}

std::vector<rectangle> find_faces_npd(npd::npddetect& npd, cv::Mat& in, double scalef, int score_thresh)
{
 std::vector<rectangle> faces;

 cv::Mat img;
 cv::cvtColor(in, img, cv::COLOR_BGR2GRAY);
 cv::resize(img, img, cv::Size(), scalef, scalef);

 double t = (double)cvGetTickCount();
 // Detect faces with open NPD 
 int n = npd.detect(img.data, img.cols, img.rows);
 /*
 if (n == 0)
 {				
   cout << "Face detector(NPD): none face detected. Skipping frame!!! \n" << endl;				
   continue;					
 }*/
 t = ((double)cvGetTickCount() - t) / ((double)cvGetTickFrequency()*1000.);
 std::vector<int> Xs = npd.getXs();
 std::vector<int> Ys = npd.getYs();
 std::vector<int> Ss = npd.getSs();
 std::vector<float> Scores = npd.getScores();

 for (int i = 0; i < n; i++)
 {
	rectangle fbox;
	//filter flag for box partitioning
	if (Scores[i] < score_thresh)
		continue;								

	//convert coordinates to original (non-scaled) image
	int center_x = (Xs[i] + Ss[i] / 2) / scalef;
	int center_y = (Ys[i] + Ss[i] / 2) / scalef;
	//Ss[i] = (Ss[i] * scale_bbox) / scalef;
	Ss[i] = Ss[i] / scalef;
	Xs[i] = center_x - Ss[i] / 2;
	Ys[i] = center_y - Ss[i] / 2;

	cv::rectangle(in, cv::Rect(Xs[i], Ys[i], Ss[i], Ss[i]), cv::Scalar(0, 0, 255));	
	
	//dlib faces rectangle construction
	fbox.set_left(Xs[i]);
	fbox.set_top(Ys[i]);
	fbox.set_right(Xs[i] + Ss[i]);
	fbox.set_bottom(Ys[i] + Ss[i]);

	faces.push_back(fbox);
  }
			
return faces;
}	

int main(int argc, char** argv)
{
	double scalef = 0.5; //0.3;       //фактор сужения картинки для поиска лица мин. размера
	int score_thresh = 60;     // //100; // 50;  //ограничивающее число для локализации face box
						     //score число очков, набранное алгоритмом NPD
	double scale_bbox = 1.35;  //эффективн. размер bbox для лучшего landmark fitting

	
	cout << "MySQL client version: " << mysql_get_client_info() << endl;
		
	MYSQL *connection = mysql_init(NULL);
	if (connection == NULL) 
	{
	  cout << "Unable to create MySQL connection" << endl;
	  return -1;	  
	}
	
	//Connecting to the MySQL database
	if (mysql_real_connect(connection, "localhost", "root", "i20cvpass", "video", 0, NULL, 0) == NULL)
	{
	  fprintf(stderr, "%s\n", mysql_error(connection));
	  mysql_close(connection);
	  return -1;
	}  
	
	cout << "Connected success: " << endl;
	
	try
	{
        cv::VideoCapture* capture;
		cv::VideoCapture  cap;

		if(argc < 4)
		    capture = new cv::VideoCapture(atoi(argv[1])); //using standart system camera 
		else
		    capture = new cv::VideoCapture(argv[3]);
		
		cout << "Video capture created " << endl;
		capture->cv::VideoCapture::set(cv::CAP_PROP_FRAME_WIDTH, 1280);
		capture->cv::VideoCapture::set(cv::CAP_PROP_FRAME_HEIGHT, 720);
		cout << "Property set success " << endl;
		cap = *capture;

		if (!cap.isOpened())
		{
		cerr << "Unable to open video source from argv[1]" << endl;
			delete capture;
			return -1;
		}

		cout << "Video capture opened " << endl;
		
		/*
		if (argc < 3) {
			cout << "Need Face DETECTOR model file at argv[2]" << endl;
			delete capture;
			return -1;
		}

		if (argc < 4) {
			cout << "Need Dlib shape predictor model file at argv[3]" << endl;
			delete capture;
			return -1;
		}*/

		/*
		if (argc < 5) {
			cout << "Need face descriptors file at argv[4]" << endl;
			delete capture;
			return -1;
		}*/


		image_window win;
		image_window win_faces;
		//image_window win_result;

		//NPD face detector init		
		npd::npddetect npd;
		npd.load("frontal_face_detector.bin");
		cout << "npd model loaded " << endl;

	        // Load face detection and pose estimation models.
	        frontal_face_detector detector = dlib::get_frontal_face_detector();
	        shape_predictor pose_model;
	        dlib::deserialize("shape_predictor_5_face_landmarks.dat") >> pose_model;
		cout << "shape_predictor_5_face_landmarks deserialized " << endl;

		//Load DNN face descriptor generated by authorization util		
//		std::vector<matrix<float, 0, 1>> in_descriptors;
		/*
		for (int i = 4; i < argc; i++) 
		{
			dlib::matrix<float, 0, 1> descriptor;
			dlib::deserialize(argv[i]) >> descriptor;
			in_descriptors.push_back(descriptor);
			cout << "Face descriptor loaded: " << trans(descriptor) << endl;
		}*/
		
		//Load DNN face descriptor from mysql database
		
		if (mysql_query(connection, "SELECT * FROM person")) finish_with_error(connection);		
		MYSQL_RES *result = mysql_store_result(connection);
		if (result == NULL) finish_with_error(connection);
		cout << "SELECT Query sucess " << endl;

		
		int num_fields = mysql_num_fields(result);
		MYSQL_ROW row;  
		//Load DNN face descriptor generated by authorization util		
		std::vector<matrix<float, 0, 1>> in_descriptors;
		std::vector<int> Ids;
		/*
		while ((row = mysql_fetch_row(result))) 	
		{ 	  
		  dlib::matrix<float, 0, 1> descriptor;
		  std::stringbuf buf(row[1]);
		  std::istream descriptor_stream(&buf);
		  dlib::deserialize(descriptor, descriptor_stream);
		  
		  in_descriptors.push_back(descriptor);
		  cout << "Face descriptor loaded: " << trans(descriptor) << endl;		  	  
		}
		mysql_free_result(result);
		cout << "FACE descriptors loaded " << endl;
		*/
		
		while ((row = mysql_fetch_row(result))) 	
		{ 	  
		  dlib::matrix<float, 0, 1> descriptor;
		  //std::stringbuf buf(row[1]);
		  std::string json_string(row[1]);
		  std::string id_string(row[0]);
		  int id = atoi(id_string.c_str());
		  Ids.push_back(id);
		
		  Json::Value test_root;
		  Json::Reader reader;
		  bool parsingSuccessful = reader.parse( json_string.c_str(), test_root );     //parse process
		  
		  if ( !parsingSuccessful )
		  {
		    std::cout  << "Failed to parse" << reader.getFormattedErrorMessages();
		    //return 0;
		  }	
	
		  std::cout << test_root.get("task_type", " " ).asString() << std::endl;
		  //std::cout << root.get("descriptor_v", " ") << std::endl;
		  
		  dlib::array<float> float_array = dlib::array<float>();
		  
		  for(int i = 0; i < 128; i++)
		  {
		    std::string x_str = test_root["descriptor_v"][i].asString();
		    float x = atof(x_str.c_str());
		    float_array.push_back(x);
		   //std::cout << x << endl;
		   //std::cout << test_root["descriptor_v"][i] << endl;
		  }
		  
		  auto float_mat = mat(float_array);
		  descriptor = dlib::matrix<float, 0, 1>(float_mat);

		  in_descriptors.push_back(descriptor);
		  cout << "Face descriptor loaded: " << trans(descriptor) << endl;
		}
		mysql_free_result(result);
		cout << "FACE descriptors loaded " << endl;
		
		// And finally we load the DNN responsible for face recognition.
		anet_type net;
		dlib::deserialize("dlib_face_recognition_resnet_model_v1.dat") >> net;

	 double fps = 10.0; //plaseholder
	 int frame_count = 0;
	 double frame_time = (double)cv::getTickCount();
        
	 cout << "Connected to video-rec channel" << endl;

	 int broadcast_id = atoi(argv[2]);
	 
	 AMQP amqp("video-rec:EcZupYb@195.211.7.218:50072"); // all connect string	 
	 AMQPExchange * ex = amqp.createExchange("common_exchange");
	 ex->Declare("common_exchange", "direct");		
	 AMQPQueue * qu2 = amqp.createQueue();
		
	 qu2->Declare("update_video_scenes", AMQP_DURABLE);
	 qu2->Bind( "common_exchange", "");		

	 ex->setHeader("Delivery-mode", 2);
	 ex->setHeader("Content-type", "text/text");
	 ex->setHeader("Content-encoding", "UTF-8");
 
	 // Grab and process frames until the main window is closed by the user.
        while(!win.is_closed())
        {
            // Grab a frame
            //cv::Mat temp;
	    cv::Mat in, frame, img, frame_proc;
	    if ( frame_count == 0 )
		frame_time = (double)cv::getTickCount();

            if (!cap.read(in))     
                break;
            
	    in.copyTo(frame);

            // Turn OpenCV's Mat into something dlib can deal with.  Note that this just
            // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
            // long as temp is valid.  Also don't do anything to temp that would cause it
            // to reallocate the memory which stores the image as that will make cimg
            // contain dangling pointers.  This basically means you shouldn't modify temp
            // while using cimg.
            cv_image<bgr_pixel> cimg(in);
	    cv_image<bgr_pixel> fimg(frame);

	    cv::putText(in, cv::format("fps %.2f",fps), cv::Point(50, 100), cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(0, 255, 255), 3);
	    
			std::vector<rectangle> faces;			
			//NPD face detector
			//faces = find_faces_npd(npd, in, scalef, score_thresh);			
			//cout << "Face detector(NPD) faces found:" << faces.size() << " time:" << t << "ms\n";
			//HOG face detector as alternative
			
			faces = detector(cimg);
			
			if (faces.size() == 0)
			{
				cout << "Face detector: none face detected. Skipping frame!!! \n" << endl;
				std::string notifier("No one known face");
				cv::Point textPoint(10, 50);
				cv::putText(in, notifier, textPoint, cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar::all(250), 2, 8);
				
				//need to publish even waste string!!!
				
				//win.clear_overlay();
				//win.set_image(cimg);
				//continue;
			}


			// Find the pose of each face.
			std::vector<full_object_detection> shapes;
			std::vector<matrix<rgb_pixel>> face_chips;
			for (unsigned long i = 0; i < faces.size(); ++i) 
			{
				auto shape = pose_model(fimg, faces[i]);
				matrix<rgb_pixel> face_chip;
				extract_image_chip(fimg, get_face_chip_details(shape, 150, 0.25), face_chip);
				face_chips.push_back(move(face_chip));
				shapes.push_back(shape);
			}
	  
			std::vector<int> id_person;
			std::vector<matrix<float, 0, 1>> face_descriptors = net(face_chips);
			int detected_idx = -1;
			cout << "INPUT Face descriptors size = " << in_descriptors.size() << endl;
			cout << "FOUND Face descriptor size = " << face_descriptors.size() << endl;
			
			//searching for close descriptors			
			for (int i = 0; i < in_descriptors.size(); i++)
			{
				for (int j = 0; j < face_descriptors.size(); j++)
				{
					double L = length(in_descriptors[i] - face_descriptors[j]);
					int x = faces[j].left();
					int y = faces[j].top();
					int x1 = faces[j].right();
					int y1 = faces[j].bottom();

					if (L < 0.6)
					{
						detected_idx = i; //saving in_descriptor number
						//сохранить detected_id в массиве обнаруженных idшников
						id_person.push_back(Ids[detected_idx]);
						
						std::string notifier("Known face detected");
						cv::Point textPoint(10, 50);
						cv::putText(in, notifier, textPoint, cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar::all(250), 2, 8);
						//cout << "Known face detected!!! Face ID = " << j << endl;
						//printing users id near detected face on a frame
						textPoint = cv::Point(x, y);
						std::string user_id(std::to_string(i)); //i - is the id_user found						
						cv::putText(in, user_id, textPoint, cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar::all(250), 3, 8);
						cv::rectangle(in, cv::Rect(x,y,x1-x,y1-y), cv::Scalar::all(255), 2);
						
						
					} else 
					{ 
					  //подсветить неизвестных чувачковъ
					  cv::rectangle(in, cv::Rect(x,y,x1-x,y1-y), cv::Scalar(120,120,120), 1);
					}
				}
			}
						 

			//if (detected_id < 0)
			if(!id_person.size())
			{
				//cout << "NO ONE KNOWN FACE... skipping" << endl;
				std::string notifier("NO ONE KNOWN FACE...");
				cv::Point textPoint(10, 50);
				cv::putText(in, notifier, textPoint, cv::FONT_HERSHEY_PLAIN, 1.5, cv::Scalar::all(150), 2, 8);
				
				//win.clear_overlay();
				//win.set_image(cimg);
				//continue;
			}
			
			//TODO: сформировать сообщение по формату 
			//запаковать его через jsoncpp
			Json::Value root;
			root["task_type"] = "update_video_scenes";
			root["task_values"]["time"] = std::time(nullptr);
			root["task_values"]["person_ids"] = Json::Value(Json::arrayValue);
			for(int i=0; i<id_person.size(); i++)			  
			  root["task_values"]["person_ids"].append(id_person[i]);			
			root["task_values"]["broadcast_id"] = broadcast_id;			
						
			//With amqp-cpp 2.1 works slowly
			/*
			if(frame_count%10 == 0)
			{
			  cout << "AMQP message" << endl;			  
			  
			  SimplePocoHandler handler("195.211.7.218", 50072);
			  AMQP::Connection amqp_connection(&handler, AMQP::Login("video-rec", "EcZupYb"), "/");
			  AMQP::Channel channel(&amqp_connection);

			  channel.declareExchange("common_exchange",AMQP::direct).onSuccess([&]{
			      if(handler.connected())
			      {
				Json::FastWriter fastWriter;
				std::string output = fastWriter.write(root);
				cout << output << endl;
				//cout << root << endl;
				channel.publish("common_exchange","update_video_scenes", output);
				handler.quit();
			       }
			      });
			 handler.loop();
			}*/
			
			//amqpcpp another wrapper
			if(frame_count%10 == 0)
			{
			  cout << "AMQP message" << endl;			  
			  Json::FastWriter fastWriter;
			  std::string output = fastWriter.write(root);
			  ex->Publish(output.c_str(), "");
			  cout << output << endl;			  
			}			
			
			win_faces.set_title("Detected faces");
			win_faces.set_image(tile_images(face_chips));

            // Display it all on the screen
            win.clear_overlay();
            win.set_image(cimg);
            win.add_overlay(render_face_detections(shapes));
	    
	    frame_count++;	    
	    if ( frame_count == 60)
            {
                frame_time = ((double)cv::getTickCount() - frame_time)/cv::getTickFrequency();
                fps = 60.0/frame_time;
                frame_count = 0;
	    }
        }
		mysql_close(connection);
		delete capture;
		return 0;
	}
    
  catch(serialization_error& e)
    {
        cout << "Deserialization ERROR!" << endl;
        //cout << "You can get it from the following URL: " << endl;
        //cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
        cout << endl << e.what() << endl;
    }
    
	catch(exception& e)
    {
        cout << e.what() << endl;
    }
}



#ifndef _FILT_HPP_
#define _FILT_HPP_

#include <iostream>

#include "blur.hpp"
#include "unique.hpp"
#include "splat.hpp"
#include "slice.hpp"
#include "testslib.hpp"
#include "csrmatrix.hpp"
#include "hashcoords.hpp"
#include "bistochastize.hpp"
#include "getvalididx.hpp"
#include "factorization.hpp"

#include<opencv2/core/core.hpp>
#include<opencv2/core/eigen.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>

#include <Eigen/Dense>
#include <Eigen/SparseCore>
#include <Eigen/SparseCholesky>
#include <Eigen/IterativeLinearSolvers>
#include <Eigen/Sparse>


    void solve(Eigen::MatrixXf& x,
               Eigen::MatrixXf& w,
               Eigen::MatrixXf& out)
    {

        clock_t now;

        Eigen::SparseMatrix<float> bluredDn(nvertices,nvertices);
        Blur(Dn,bluredDn);
	    // std::cout << "start Blur(Dn,bluredDn)" << std::endl;
        Eigen::SparseMatrix<float> A_smooth = Dm - Dn * bluredDn;
        now = clock();
        printf( "A_smooth : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);


        // SparseMatrix<float> A_diag(nvertices);
        Eigen::SparseMatrix<float> M(nvertices,nvertices);
        Eigen::SparseMatrix<float> A_data(nvertices,nvertices);
        Eigen::SparseMatrix<float> A(nvertices,nvertices);
        Eigen::VectorXf b(nvertices);
        Eigen::VectorXf y(nvertices);
        Eigen::VectorXf w_splat(nvertices);
        Eigen::VectorXf xw(x.size());


	    // std::cout << "start Splat(w,w_splat)" << std::endl;
        Splat(w,w_splat);
        diags(w_splat,A_data);
        A = bs_param.lam * A_smooth + A_data ;
        now = clock();
        printf( "A : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);

        xw = x.array() * w.array();

        Splat(xw,b);
        now = clock();
        printf( "b : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);


        std::cout << "solve" << std::endl;
        // fill A and b
        Eigen::ConjugateGradient<Eigen::SparseMatrix<float>, Eigen::Lower|Eigen::Upper> cg;
        cg.compute(A);
        for (size_t i = 0; i < bs_param.cg_maxiter; i++) {
            y = cg.solve(b);
            std::cout << "#iterations:     " << cg.iterations() << std::endl;
            std::cout << "estimated error: " << cg.error()      << std::endl;
            if(cg.error()  < bs_param.cg_tol) break;
        }
        now = clock();
        printf( "solved : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);

        Slice(y,out);
        now = clock();
        printf( "Sliced : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);



    }




    // void solve(std::vector<float>& x, std::vector<float>& w,std::vector<float>& out)
    // {
    //
    //     clock_t now;
    //
    //     Eigen::SparseMatrix<float> bluredDn(nvertices,nvertices);
    //     Blur(Dn,bluredDn);
	//     // std::cout << "start Blur(Dn,bluredDn)" << std::endl;
    //     Eigen::SparseMatrix<float> A_smooth = Dm - Dn * bluredDn;
    //     now = clock();
    //     printf( "A_smooth : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);
    //
    //
    //     // SparseMatrix<float> A_diag(nvertices);
    //     Eigen::SparseMatrix<float> M(nvertices,nvertices);
    //     Eigen::SparseMatrix<float> A_data(nvertices,nvertices);
    //     Eigen::SparseMatrix<float> A(nvertices,nvertices);
    //     Eigen::VectorXf b(nvertices);
    //     Eigen::VectorXf y(nvertices);
    //     std::vector<float> w_splat;
    //     std::vector<float> xw(x.size());
    //     std::vector<float> y0;
    //     std::vector<float> yhat;
    //
    //
	//     // std::cout << "start Splat(w,w_splat)" << std::endl;
    //     Splat(w,w_splat);
    //     diags(w_splat,A_data);
    //     A = bs_param.lam * A_smooth + A_data ;
    //     now = clock();
    //     printf( "A : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);
    //
    //
    //     for (int i = 0; i < x.size(); i++) {
    //         xw[i] = x[i] * w[i];
    //     }
	//     // std::cout << "start Splat(xw,b)" << std::endl;
    //     Splat(xw,b);
    //     now = clock();
    //     printf( "b : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);
    //
    //     // std::cout << "Construct M" << std::endl;
    //     // for (int i = 0; i < nvertices; i++) {
    //     //     if(A.coeff(i,i) > bs_param.A_diag_min)
    //     //     {
    //     //         M.insert(i,i) = 1.0/A.coeff(i,i);
    //     //     }
    //     //     else
    //     //     {
    //     //         M.insert(i,i) = 1.0/bs_param.A_diag_min;
    //     //     }
    //     // }
    //
    //     // std::cout << "Construct y0" << std::endl;
    //     // y0.resize(b.size());
    //     // for (int i = 0; i < b.size(); i++) {
    //     //     y0[i] = b(i) / w_splat[i];
    //     // }
    //     // yhat = y0;       // why shold empty_like(y0)
    //
    //     std::cout << "solve" << std::endl;
    //     // fill A and b
    //     Eigen::ConjugateGradient<Eigen::SparseMatrix<float>, Eigen::Lower|Eigen::Upper> cg;
    //     cg.compute(A);
    //     for (size_t i = 0; i < bs_param.cg_maxiter; i++) {
    //         y = cg.solve(b);
    //         std::cout << "#iterations:     " << cg.iterations() << std::endl;
    //         std::cout << "estimated error: " << cg.error()      << std::endl;
    //         if(cg.error()  < bs_param.cg_tol) break;
    //     }
    //     now = clock();
    //     printf( "solved : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);
    //
    //     Slice(y,out);
    //     now = clock();
    //     printf( "Sliced : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);
    //
    //
    //
    // }


    void test_solve()
    {

        std::cout << "hello solver" << '\n';

        clock_t now;
        now = clock();
        printf( "start : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);

        cv::Mat reference = cv::imread("reference.png");
        cv::Mat im1 = cv::imread("reference.png");
        cv::Mat target = cv::imread("target.png");
        cv::Mat confidence = cv::imread("confidence.png");

        // cv::Mat reference = cv::imread("rgb.png");
        // cv::Mat im1 = cv::imread("rgb.png");
        // cv::Mat target = cv::imread("depth.png");
        // cv::Mat confidence = cv::imread("depth.png");
        // cv::Mat target = cv::imread(argv[2],0);

    	cvtColor(reference, reference, cv::COLOR_BGR2YCrCb);
    	cvtColor(target, target, cv::COLOR_BGR2YCrCb);
    	cvtColor(confidence, confidence, cv::COLOR_BGR2YCrCb);

        std::cout << "reference:" << reference.cols<<"x"<< reference.rows<< std::endl;


        float spatialSigma = 250.0;
        float lumaSigma = 8.0;
        float chromaSigma = 4.0;

        npixels = reference.cols*reference.rows;

        cv::Mat r(npixels, 5, CV_64F);
        cv::Mat t(npixels, 1, CV_64F);
        cv::Mat c(npixels, 1, CV_64F);
        // std::vector<float> re(reference.cols*reference.rows*5);
        // std::vector<float> ta(reference.cols*reference.rows);
        // std::vector<float> co(reference.cols*reference.rows);
        int idx = 0;
    	std::cout << "start filling positions and values" << std::endl;
        now = clock();
        printf( "fill positions and values : now is %f seconds\n", (float)(now) / CLOCKS_PER_SEC);
        for (int y = 0; y < reference.cols; y++) {
            for (int x = 0; x < reference.rows; x++) {
                float *datar = r.ptr<float>(idx);
                datar[0] = ceilf(x/spatialSigma);
                datar[1] = ceilf(y/spatialSigma);
                datar[2] = ceilf(reference.at<cv::Vec3b>(x,y)[0]/lumaSigma);
                datar[3] = ceilf(reference.at<cv::Vec3b>(x,y)[1]/chromaSigma);
                datar[4] = ceilf(reference.at<cv::Vec3b>(x,y)[2]/chromaSigma);
                // datar[3] = 1.0;
                // datar[4] = 1.0;
                idx++;
            }
        }
        idx = 0;
        for (int y = 0; y < reference.cols; y++) {
            for (int x = 0; x < reference.rows; x++) {
                float *datac = c.ptr<float>(idx);
                // datac[0] = 1.0;
                datac[0] = confidence.at<cv::Vec3b>(x,y)[0];
                idx++;
            }
        }
        idx = 0;
        for (int y = 0; y < reference.cols; y++) {
            for (int x = 0; x < reference.rows; x++) {
                float *datat = t.ptr<float>(idx);
                datat[0] = target.at<cv::Vec3b>(x,y)[0];
                idx++;
            }
        }

        std::cout << "cv2eigen" << std::endl;
        Eigen::MatrixXf ref;
        Eigen::MatrixXf tar;
        Eigen::MatrixXf con;

        cv::cv2eigen(r,ref);
        cv::cv2eigen(t,tar);
        cv::cv2eigen(c,con);
        std::cout << "finished cv2eigen" << std::endl;


    	// std::cout << "start filling positions and values" << std::endl;
        // now = clock();
        // printf( "fill positions and values : now is %f seconds\n", (float)(now) / CLOCKS_PER_SEC);
        // idx = 0;
        // for (int y = 0; y < reference.cols; y++) {
        //     for (int x = 0; x < reference.rows; x++) {
        //         re[idx*5+0] = ceilf(x/spatialSigma);
        //         re[idx*5+1] = ceilf(y/spatialSigma);
        //         re[idx*5+2] = ceilf(reference.at<cv::Vec3b>(x,y)[0]/lumaSigma);
        //         re[idx*5+3] = ceilf(reference.at<cv::Vec3b>(x,y)[1]/chromaSigma);
        //         re[idx*5+4] = ceilf(reference.at<cv::Vec3b>(x,y)[2]/chromaSigma);
        //
        //         // ref[idx*5+0] = (x/spatialSigma);
        //         // ref[idx*5+1] = (y/spatialSigma);
        //         // ref[idx*5+2] = (reference.at<cv::Vec3b>(x,y)[0]/lumaSigma);
        //         // ref[idx*5+3] = (reference.at<cv::Vec3b>(x,y)[1]/chromaSigma);
        //         // ref[idx*5+4] = (reference.at<cv::Vec3b>(x,y)[2]/chromaSigma);
        //         // tar[idx] = target.at<cv::Vec3b>(x,y)[0];
        //         // con[idx] = confidence.at<cv::Vec3b>(x,y)[0];
        //         // values[idx*4+1] = target.at<uchar>(x,y);
        //         // values[idx*4+2] = target.at<uchar>(x,y);
        //         // values[idx*4+1] = 1.0f;
        //         // values[idx*4+2] = 1.0f;
        //         // values[idx*4+3] = 1.0f;
        //         idx++;
        //     }
        // }


        now = clock();
        printf( "compute_factorization : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);
        compute_factorization(ref);
        // compute_factorization(re);

        now = clock();
        printf( "bistochastize : now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);
        bistochastize();

        now = clock();
        printf( "solve :now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);
        solve(tar,con,tar);
        now = clock();
        printf( "solved :now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);

        // Divide through by the homogeneous coordinate and store the
        // result back to the image
        idx = 0;
        for (int y = 0; y < reference.cols; y++) {
            for (int x = 0; x < reference.rows; x++) {
                // float w = values[idx*4+3];
                target.at<cv::Vec3b>(x,y)[0] = tar(idx);
                // target.at<cv::uchar>(x,y) = values[idx*4+1]/w;
                // target.at<cv::uchar>(x,y) = values[idx*4+2]/w;
                // target.at<cv::Vec3b>(x,y)[0] = values[idx*4+0]/w;
                // target.at<cv::Vec3b>(x,y)[1] = values[idx*4+0]/w;
                // target.at<cv::Vec3b>(x,y)[2] = values[idx*4+0]/w;
                idx++;
            }
        }

    	cvtColor(reference, reference, cv::COLOR_YCrCb2BGR);
    	cvtColor(target, target, cv::COLOR_YCrCb2BGR);


        now = clock();
        printf( "finished :now is %f seconds\n\n", (float)(now) / CLOCKS_PER_SEC);
    	cv::imshow("input",im1);
    	cv::imshow("output",target);
    	cv::waitKey(0);



    }



#endif //_FILT_HPP_

/*
Copyright 2005, 2006 Computer Vision Lab,
Ecole Polytechnique Federale de Lausanne (EPFL), Switzerland.
All rights reserved.

This file is part of BazAR.

BazAR is free software; you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

BazAR is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
BazAR; if not, write to the Free Software Foundation, Inc., 51 Franklin
Street, Fifth Floor, Boston, MA 02110-1301, USA
*/
#include <sys/stat.h> // for mkdir()
#include <fstream>
#include <algorithm>

#ifdef WIN32
#include <direct.h> // for _mkdir()
#endif

using namespace std;

#include <starter.h>
#include <keypoints/yape.h>
#include "planar_object_recognizer.h"

// damian
#include "../artvertiser/FProfiler/FProfiler.h"

planar_object_recognizer::planar_object_recognizer()
: forest(0), model_points(0), object_input_view(0),
model_and_input_images(0), point_detector(0), homography_estimator(0), affine_motion(0), H(0),
detected_points(0), detected_point_views(0)
{
    for(int i = 0; i < hard_max_detected_pts; i++) {
        (match_probabilities[i] = 0);
    }


    clear();



}


void planar_object_recognizer::clear()
{
    lock();

    ready = false;

    // damian: clean up multithreaded tracker stuff
    if ( affine_thread_data.size() > 0 )
    {
      // kill all the threads
      for ( int i=0; i<affine_thread_data.size(); i++ )
      {
          // tell the thread to stop
          affine_thread_data[i]->should_stop = true;
          affine_thread_data[i]->start_signal.Signal();
          // wait for completion
          void* res;
          pthread_join( affine_thread_data[i]->thread, &res );
          // delete
          delete( affine_thread_data[i] );
      }
      affine_thread_data.clear();
      delete shared_barrier;    shared_barrier = 0;
    }

    if (object_input_view)  delete object_input_view;   object_input_view = 0;
    if (point_detector)     delete point_detector;      point_detector = 0;

    if (model_points != 0)  delete [] model_points;     model_points = 0;

    if (forest != 0)        delete forest;              forest =0;

    for(int i = 0; i < hard_max_detected_pts; i++) {
    if (match_probabilities[i])
        delete [] match_probabilities[i];   match_probabilities[i] = 0;
    }

    if (detected_points) delete[] detected_points;      detected_points = 0;
    if (detected_point_views) delete[] detected_point_views;    detected_point_views = 0;
    if (affine_motion) delete affine_motion;            affine_motion = 0;
    if (H) delete H;                                    H = 0;
    if (homography_estimator) delete homography_estimator;  homography_estimator = 0;


    default_settings();

    detected_points = new keypoint[hard_max_detected_pts];
    detected_point_views = new image_class_example[hard_max_detected_pts];

    match_score_threshold = .05f;
    ransac_dist_threshold = 10;
    ransac_stop_support = 50;
    max_ransac_iterations = 800;
    non_linear_refine_threshold = 3;

    put_ui_settings();

    unlock();
}

void planar_object_recognizer::default_settings(void)
{
  set_max_depth(10);
  do_proportional_match_lut = true;

  best_support_thresh = 10;

  dont_use_bins_when_creating_model_points();
  dont_use_bins_when_detecting_input_image_points();

  index_of_model_point_to_debug = -1;

  //sample_number_for_refining = 1000;
  //views_number = 1000;
  sample_number_for_refining = 100;
  views_number = 100;

  max_detected_pts = 1000;
  min_view_rate = .4;
  keypoint_distance_threshold = 1.5;
  point_detector_tau = 10;
}

void planar_object_recognizer::set_max_detected_pts(int max)
{
  if (max > 0 && max < hard_max_detected_pts)
    max_detected_pts = max;
}

void planar_object_recognizer::use_orientation_correction(void)
{
  new_images_generator.set_use_orientation_correction(true);
}

void planar_object_recognizer::dont_use_orientation_correction(void)
{
  new_images_generator.set_use_orientation_correction(false);
}

void planar_object_recognizer::set_range_variation_for_theta(float min_theta, float max_theta)
{
  new_images_generator.set_range_variation_for_theta(min_theta, max_theta);
}

void planar_object_recognizer::set_range_variation_for_phi(float min_phi, float max_phi)
{
  new_images_generator.set_range_variation_for_phi(min_phi, max_phi);
}

void planar_object_recognizer::independent_scaling(float min_lambda1, float max_lambda1,
                                                   float min_lambda2, float max_lambda2)
{
  new_images_generator.independent_scaling(min_lambda1, max_lambda1, min_lambda2, max_lambda2);
}

void planar_object_recognizer::constrained_scaling(float min_lambda1, float max_lambda1,
                                                   float min_lambda2, float max_lambda2,
                                                   float min_l1_l2,  float max_l1_l2)
{
  new_images_generator.constrained_scaling(min_lambda1, max_lambda1,
                                           min_lambda2, max_lambda2,
                                           min_l1_l2, max_l1_l2);
}

void planar_object_recognizer::set_use_random_background(bool use_random_background)
{
  new_images_generator.set_use_random_background(use_random_background);
}

void planar_object_recognizer::set_noise_level(int noise_level)
{
  new_images_generator.set_noise_level(noise_level);
}

/*! Build a classifier from the given model image. The result is not saved.
  \return true on success, false on failure.
  */
bool planar_object_recognizer::build(IplImage *model_image,
                                                int max_point_number_on_model, int patch_size,
                                                int yape_radius, int tree_number, int nbLev,
                                                LEARNPROGRESSION LearnProgress, int *roi)
{
    put_ui_settings();


  new_images_generator.set_patch_size(patch_size);

  IplImage *model;
  if (model_image->nChannels == 1)
	  model = model_image;
  else {
	  model = cvCreateImage(cvGetSize(model_image), IPL_DEPTH_8U, 1);
	  cvCvtColor(model_image, model, CV_RGB2GRAY);
  }

  if (roi)
	  new_images_generator.set_original_image(model,
			  roi[0], roi[1],
			  roi[2], roi[3],
			  roi[4], roi[5],
			  roi[6], roi[7]);
  else
	  new_images_generator.set_original_image(model);

  if (model != model_image) cvReleaseImage(&model);

  new_images_generator.set_level_number(nbLev);
  new_images_generator.set_gaussian_smoothing_kernel_size(yape_radius);

  initialize();

  learn(max_point_number_on_model, patch_size, yape_radius, tree_number, nbLev, LearnProgress);


  ready = true;
  printf("**~ detector now ready\n");

  return true;
}


bool planar_object_recognizer::build_with_cache(string filename,
                                                int max_point_number_on_model, int patch_size,
                                                int yape_radius, int tree_number, int nbLev,
                                                LEARNPROGRESSION LearnProgress)
{
    put_ui_settings();

  new_images_generator.set_patch_size(patch_size);

  string dirname(filename + ".classifier");

  string roifn(filename + ".roi");
  ifstream roif(roifn.data());
  int u[4] = {-1, -1, -1, -1};
  int v[4] = {-1, -1, -1, -1};
  if (roif.good()) {
    cout << "Reading ROI from file " << roifn << ".\n";
    for (int i = 0; i < 4; i++) {
      roif >> u[i];
      roif >> v[i];
    }
    roif.close();
  }

/*
  roifn = string(filename)+".artvertroi";
  roif = ifstream(roifn.data());
  if ( roif.good() )
  {
    cout << "Reading artvert ROI from file " << roifn << ".\n";
    for (int i = 0; i < 4; i++) {
      roif >> u[i];
      roif >> v[i];
    }
    roif.close();
  }
*/

  IplImage * model = mcvLoadImage(filename.data(), 0);

  if (load(dirname) and model)
  {
     new_images_generator.set_original_image(model, u[0], v[0], u[1], v[1], u[2], v[2], u[3], v[3]);
     /*new_images_generator.set_level_number(nbLev);
     new_images_generator.set_gaussian_smoothing_kernel_size(yape_radius);*/
    ready = true;
    printf("**~ detector now ready A\n");
	 return true;
  }

  if (model == 0 ) return false;

  new_images_generator.set_original_image(model, u[0], v[0], u[1], v[1], u[2], v[2], u[3], v[3]);
  new_images_generator.set_level_number(nbLev);
  new_images_generator.set_gaussian_smoothing_kernel_size(yape_radius);

  initialize();

  learn(max_point_number_on_model, patch_size, yape_radius, tree_number, nbLev, LearnProgress);

  save(dirname);

  ready = true;
  printf("**~ detector now ready B\n");

  return true;
}

bool planar_object_recognizer::load(string directory_name)
{
  // Read parameters:
  char parameter_filename[1000];
  sprintf(parameter_filename, "%s/parameters.txt", directory_name.data());
  ifstream param_f(parameter_filename);
  int yape_radius;
  int nbLev;

  if (!param_f.good()) return false;
  param_f >> yape_radius;
  param_f >> nbLev;
  param_f.close();

  new_images_generator.set_level_number(nbLev);
  new_images_generator.set_gaussian_smoothing_kernel_size(yape_radius);

  // Read original image:
  char image_name[1000];
  sprintf(image_name, "%s/original_image.bmp", directory_name.data());
  IplImage * original_image = mcvLoadImage(image_name, 0);
  if (original_image == 0) return false;

  // Read object corner positions in this original image:
  char corner_filename[1000];
  sprintf(corner_filename, "%s/corners.txt", directory_name.data());
  ifstream cf(corner_filename);

  if (!cf.good()) return false;
  int u_corner1, v_corner1, u_corner2, v_corner2, u_corner3, v_corner3, u_corner4, v_corner4;
  cf >> u_corner1 >> v_corner1;
  cf >> u_corner2 >> v_corner2;
  cf >> u_corner3 >> v_corner3;
  cf >> u_corner4 >> v_corner4;
  cf.close();
  new_images_generator.set_original_image(original_image,
    u_corner1, v_corner1, u_corner2, v_corner2, u_corner3, v_corner3, u_corner4, v_corner4);
  cvReleaseImage(&original_image);

  // Read keypoints in this original image:
  char point_filename[1000];
  sprintf(point_filename, "%s/keypoints.txt", directory_name.data());
  ifstream pf(point_filename);
  if (!pf.good()) return false;
  pf >> model_point_number;
  model_points = new object_keypoint[model_point_number];

  for(int i = 0; i < model_point_number; i++) {
    pf >> model_points[i].M[0] >> model_points[i].M[1] >> model_points[i].M[2] >> model_points[i].scale;
    model_points[i].class_index = i;
    assert(model_points[i].scale < nbLev);
    if (!pf.good()) {
      cerr << "problem while reading keypoint #" << i <<" in " << point_filename << endl;
      return false;
    }
  }
  pf.close();

  new_images_generator.set_object_keypoints(model_points, model_point_number);

  // Read image classification forest for this object:
  forest = new image_classification_forest();

  if ( !forest->load(directory_name) )
  {
    delete forest;
    forest = 0;
    return false;
  }

  // Match probabilities:
  for(int i = 0; i < hard_max_detected_pts; i++)
  {
    detected_point_views[i].alloc(forest->image_width);
    match_probabilities[i] = new float[model_point_number];
  }

  point_detector = new pyr_yape(new_images_generator.original_image->width, new_images_generator.original_image->height, nbLev);
  point_detector->set_radius(yape_radius);

  initialize();

  return true;
}

void planar_object_recognizer::initialize(void)
{
  object_input_view = new object_view(new_images_generator.original_image->width,
                                      new_images_generator.original_image->height,
                                      new_images_generator.level_number);

  // For motion_estimation:
  affine_motion = new affinity();
  H = new homography();

  homography_estimator = new ls_minimizer2(8);

  // For visualization:
  model_and_input_images = 0;
}

planar_object_recognizer::~planar_object_recognizer(void)
{
    clear();

    if ( detected_points)
        delete[] detected_points;
    if ( detected_point_views )
        delete[] detected_point_views;

}

void planar_object_recognizer::learn(int max_point_number_on_model, int patch_size,
                                     int yape_radius, int tree_number, int nbLev,
                                     LEARNPROGRESSION LearnProgress)
{
  if (point_detector) delete point_detector;
  point_detector = new pyr_yape(new_images_generator.original_image->width, new_images_generator.original_image->height, nbLev);
  point_detector->set_radius(yape_radius);
  point_detector->set_use_bins(use_bins_for_model_points);

  detect_most_stable_model_points(max_point_number_on_model, patch_size, views_number, min_view_rate, LearnProgress);

  save_image_of_model_points(patch_size);

  forest = new image_classification_forest(patch_size, patch_size, model_point_number, max_depth, tree_number, LearnProgress);

  // Pick m1 and m2 at random in each node of each tree
  forest->create_trees_at_random();

  // Refine posterior probabilities for each leaf of each tree
  forest->refine(/* example_generator */ &new_images_generator, /* call number to generate_random_examples */ sample_number_for_refining);
  forest->test(/* example_generator */ &new_images_generator, /* call number to generate_random_examples */ 300);

  for(int i = 0; i < hard_max_detected_pts; i++)
  {
    detected_point_views[i].alloc(patch_size);
    match_probabilities[i] = new float[model_point_number];
  }

}

void planar_object_recognizer::save(string directory_name)
{
#ifndef WIN32
  mkdir(directory_name.data(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IWOTH);
#else
  _mkdir(directory_name.data());
#endif

  char image_name[1000];
  sprintf(image_name, "%s/original_image.bmp", directory_name.data());
  mcvSaveImage(image_name, new_images_generator.original_image);

  char corner_filename[1000];
  sprintf(corner_filename, "%s/corners.txt", directory_name.data());
  ofstream cf(corner_filename);
  cf << new_images_generator.u_corner1 << " " << new_images_generator.v_corner1 << endl;
  cf << new_images_generator.u_corner2 << " " << new_images_generator.v_corner2 << endl;
  cf << new_images_generator.u_corner3 << " " << new_images_generator.v_corner3 << endl;
  cf << new_images_generator.u_corner4 << " " << new_images_generator.v_corner4 << endl;
  cf.close();

  char parameter_filename[1000];
  sprintf(parameter_filename, "%s/parameters.txt", directory_name.data());
  ofstream param_f(parameter_filename);
  param_f << point_detector->get_radius() << endl;
  param_f << new_images_generator.level_number << endl;
  param_f.close();

  char point_filename[1000];
  sprintf(point_filename, "%s/keypoints.txt", directory_name.data());
  ofstream pf(point_filename);
  pf << model_point_number << endl;
  for(int i = 0; i < model_point_number; i++)
    pf << model_points[i].M[0] << " " << model_points[i].M[1]
    << " " << model_points[i].M[2]
    << " " << model_points[i].scale << endl;
  pf.close();

  forest->save(directory_name);
}

void planar_object_recognizer::detect_points(IplImage * input_image)
{
  check_target_size(input_image);
  point_detector->set_use_bins(use_bins_for_input_image);
  point_detector->set_tau(point_detector_tau);

  detected_point_number = point_detector->pyramidBlurDetect(input_image,
                                                            detected_points, max_detected_pts,
                                                            &object_input_view->image);
  //printf("found %i points\n", detected_point_number );
}

void planar_object_recognizer::preprocess_points(void)
{
  int patch_size = forest->image_width;

  //object_input_view->comp_gradient();
  object_input_view->comp_gradient_mt();

  PROFILE_SECTION_PUSH("point stuff loop");
  for(int i = 0; i < detected_point_number; i++)
  {
    image_class_example * pv = &(detected_point_views[i]);
    pv->point2d = &(detected_points[i]);
    float u = pv->point2d->u, v = pv->point2d->v;
    int s = int(pv->point2d->scale);

    if (u > (patch_size/2) && u < object_input_view->image[s]->width - (patch_size/2) &&
        v > (patch_size/2) && v < object_input_view->image[s]->height - (patch_size/2))
    {
      new_images_generator.preprocess_point_view(pv, object_input_view);

      //if (0) mcvSaveImage("patches/detected_%03d_corrected.bmp", i, pv->preprocessed);
    }
  }
  PROFILE_SECTION_POP();
}

void planar_object_recognizer::match_points(bool fill_match_struct)
{
  match_number = 0;

  int patch_size = forest->image_width;

  for(int i = 0; i < detected_point_number; i++)
  {
    image_class_example * pv = &(detected_point_views[i]);
    float u = pv->point2d->u, v = pv->point2d->v;
    int s = int(pv->point2d->scale);

    if (u > (patch_size/2) && u < object_input_view->image[s]->width - (patch_size/2) &&
        v > (patch_size/2) && v < object_input_view->image[s]->height - (patch_size/2))
    {
        // calculate probabilities + put into match_probabilities[i]
        forest->posterior_probabilities(pv, match_probabilities[i]);


      if (fill_match_struct) {
        int model_point_index = 0;
        float score = match_probabilities[i][model_point_index];
        for(int j = 1; j < model_point_number; j++)
          if (match_probabilities[i][j] > score)
          {
            score = match_probabilities[i][j];
            model_point_index = j;
          }
          image_object_point_match * match = &(matches[match_number]);
          match->image_point = pv->point2d;
          match->object_point = &(model_points[model_point_index]);
          match->score = score;

          match_number++;
      }
    }
    else
      memset(match_probabilities[i], 0, sizeof(float) * model_point_number);
  }
}

bool planar_object_recognizer::detect(IplImage * input_image)
{
    get_ui_settings();

    PROFILE_SECTION_PUSH("detect points");
    detect_points(input_image);
    PROFILE_SECTION_POP();
    PROFILE_SECTION_PUSH("preprocess points");
    preprocess_points();
    PROFILE_SECTION_POP();

    PROFILE_SECTION_PUSH("match points");
    match_points();
    PROFILE_SECTION_POP();

    object_is_detected = false;

    PROFILE_SECTION_PUSH("estimate affine");

    //object_is_detected = estimate_affine_transformation(); // Estimation of "affine_motion" using Ransac.
    object_is_detected = estimate_affine_transformation_mt();

    affine_motion->transform_point(float(new_images_generator.u_corner1),
                                 float(new_images_generator.v_corner1), &detected_u_corner1, &detected_v_corner1);
    affine_motion->transform_point(float(new_images_generator.u_corner2),
                                 float(new_images_generator.v_corner2), &detected_u_corner2, &detected_v_corner2);
    affine_motion->transform_point(float(new_images_generator.u_corner3),
                                 float(new_images_generator.v_corner3), &detected_u_corner3, &detected_v_corner3);
    affine_motion->transform_point(float(new_images_generator.u_corner4),
                                 float(new_images_generator.v_corner4), &detected_u_corner4, &detected_v_corner4);

    if (object_is_detected)
    {
        object_is_detected = estimate_homographic_transformation_nonlinear_method();

        if (object_is_detected)
        {
            H->transform_point(float(new_images_generator.u_corner1),
                         float(new_images_generator.v_corner1), &detected_u_corner1, &detected_v_corner1);
            H->transform_point(float(new_images_generator.u_corner2),
                         float(new_images_generator.v_corner2), &detected_u_corner2, &detected_v_corner2);
            H->transform_point(float(new_images_generator.u_corner3),
                         float(new_images_generator.v_corner3), &detected_u_corner3, &detected_v_corner3);
            H->transform_point(float(new_images_generator.u_corner4),
                         float(new_images_generator.v_corner4), &detected_u_corner4, &detected_v_corner4);

            //cout << "-------------------------" << endl;
            //cout << *H << endl;
        }

        object_is_detected = true;
        /*
        cout << detected_u_corner1 << " x " << detected_v_corner1 << ", "
            << detected_u_corner2 << " x " << detected_v_corner2 << ", "
            << detected_u_corner3 << " x " << detected_v_corner3 << ", "
            << detected_u_corner4 << " x " << detected_v_corner4 << endl;
        */
    }

    PROFILE_SECTION_POP();

    return object_is_detected;
}

//
// ESTIMATING THE MOTION: ///////////////////////////////////////////////////////////////////////////////////////////////
//


bool planar_object_recognizer::three_random_correspondences(int * n1, int * n2, int * n3)
{

  if (match_number == 0)
    return false;

  int shot = 0;
  do
  {
    //*n1 = rand() % match_number;
    *n1 = match_index_lookup[rand()%MATCH_LOOKUP_TABLE_SIZE];
    shot++; if (shot > 100) return false;
  }
  while(matches[*n1].score < match_score_threshold);

  shot = 0;
  do
  {
    *n2 = match_index_lookup[rand()%MATCH_LOOKUP_TABLE_SIZE];//rand() % match_number;
    shot++; if (shot > 100) return false;
  }
  while(matches[*n2].score < match_score_threshold || *n2 == *n1);

  shot = 0;
  do
  {
    *n3 = match_index_lookup[rand()%MATCH_LOOKUP_TABLE_SIZE];//rand() % match_number;
    shot++; if (shot > 100) return false;
  }
  while(matches[*n3].score < match_score_threshold || *n3 == *n1 || *n3 == *n2);

  return true;
}

#define RANSAC_DIST_THRESHOLD ransac_dist_threshold

int planar_object_recognizer::compute_support_for_affine_transformation_readonly(affinity * A)
{
  int result = 0;

  if (!valid(A))
  {
    return 0;
  }

  for(int i = 0; i < match_number; i++)
  {
    image_object_point_match * match = &(matches[i]);

    float mu = PyrImage::convCoordf(float(match->object_point->M[0]), int(match->object_point->scale), 0);
    float mv = PyrImage::convCoordf(float(match->object_point->M[1]), int(match->object_point->scale), 0);

    float du = PyrImage::convCoordf(match->image_point->u, int(match->image_point->scale), 0);
    float dv = PyrImage::convCoordf(match->image_point->v, int(match->image_point->scale), 0);
    float tu, tv;

    A->transform_point(mu, mv, &tu, &tv);

    if ((du - tu) * (du - tu) + (dv - tv) * (dv - tv) < RANSAC_DIST_THRESHOLD * RANSAC_DIST_THRESHOLD)
    {
      result++;
    }
  }

  return result;
}


int planar_object_recognizer::compute_support_for_affine_transformation(affinity * A)
{
  int result = 0;

  if (!valid(A))
  {
    for(int i = 0; i < match_number; i++)
      matches[i].inlier = false;

    return 0;
  }

  for(int i = 0; i < match_number; i++)
  {
    image_object_point_match * match = &(matches[i]);

    float mu = PyrImage::convCoordf(float(match->object_point->M[0]), int(match->object_point->scale), 0);
    float mv = PyrImage::convCoordf(float(match->object_point->M[1]), int(match->object_point->scale), 0);

    float du = PyrImage::convCoordf(match->image_point->u, int(match->image_point->scale), 0);
    float dv = PyrImage::convCoordf(match->image_point->v, int(match->image_point->scale), 0);
    float tu, tv;

    A->transform_point(mu, mv, &tu, &tv);

    if ((du - tu) * (du - tu) + (dv - tv) * (dv - tv) < RANSAC_DIST_THRESHOLD * RANSAC_DIST_THRESHOLD)
    {
      result++;
      match->inlier = true;
    }
    else
      match->inlier = false;
  }

  return result;
}

bool planar_object_recognizer::valid(affinity * A)
{
  float det = float(cvmGet(A, 0, 0) * cvmGet(A, 1, 1) - cvmGet(A, 1, 0) * cvmGet(A, 0, 1));

  if (det < 0. || det > 4 * 4)
    return false;

  return true;
}






void* planar_object_recognizer::estimate_affine_transformation_thread_func( void* _data )
{
    //PROFILE_THIS_FUNCTION();

    EstimateAffineThreadData* data = (EstimateAffineThreadData*)_data;

    planar_object_recognizer* detector = data->detector;

    while( true )
    {
        // wait for signal to begin
        data->start_signal.Wait();
        if ( data->should_stop )
            break;

        //printf("thread %i running, %i iterations\n", data->thread_id, data->num_ransac_iterations );

        // construct random correspondencies
        int randoms[3*data->num_ransac_iterations];
        int actual_ransac_iterations = data->num_ransac_iterations;
        {
            //PROFILE_THIS_BLOCK("three random");
            bool three_random = true;
            int i;
            for ( i=0; i < data->num_ransac_iterations && three_random; i++ )
            {
                // create 3 random correspondencies, bail if three_random_corr returns false
                three_random &= detector->three_random_correspondences( &randoms[3*i], &randoms[3*i+1], &randoms[3*i+2] );
            }
            // in case three_random_correpsondencies fails early, we store how many we actually have
            actual_ransac_iterations = i-1;
        }


        // transform points
        //printf("actual_ransac_iterations %i\n", actual_ransac_iterations);
        float transformed_points[4*3*actual_ransac_iterations];
        {
            //PROFILE_THIS_BLOCK( "transform" );
            for ( int i=0; i<actual_ransac_iterations; i++ )
            {
                image_object_point_match *m1 = detector->matches + randoms[3*i+0];
                image_object_point_match *m2 = detector->matches + randoms[3*i+1];
                image_object_point_match *m3 = detector->matches + randoms[3*i+2];

                transformed_points[4*3*i+0 ] = PyrImage::convCoordf(float(m1->object_point->M[0]), int(m1->object_point->scale), 0);
                transformed_points[4*3*i+1 ] = PyrImage::convCoordf(float(m1->object_point->M[1]), int(m1->object_point->scale), 0);
                transformed_points[4*3*i+2 ] = PyrImage::convCoordf(float(m1->image_point->u), int(m1->image_point->scale), 0);
                transformed_points[4*3*i+3 ] = PyrImage::convCoordf(float(m1->image_point->v), int(m1->image_point->scale), 0);

                transformed_points[4*3*i+4 ] = PyrImage::convCoordf(float(m2->object_point->M[0]), int(m2->object_point->scale), 0);
                transformed_points[4*3*i+5 ] = PyrImage::convCoordf(float(m2->object_point->M[1]), int(m2->object_point->scale), 0);
                transformed_points[4*3*i+6 ] = PyrImage::convCoordf(float(m2->image_point->u), int(m2->image_point->scale), 0);
                transformed_points[4*3*i+7 ] = PyrImage::convCoordf(float(m2->image_point->v), int(m2->image_point->scale), 0);

                transformed_points[4*3*i+8 ] = PyrImage::convCoordf(float(m3->object_point->M[0]), int(m3->object_point->scale), 0);
                transformed_points[4*3*i+9 ] = PyrImage::convCoordf(float(m3->object_point->M[1]), int(m3->object_point->scale), 0);
                transformed_points[4*3*i+10] = PyrImage::convCoordf(float(m3->image_point->u), int(m3->image_point->scale), 0);
                transformed_points[4*3*i+11] = PyrImage::convCoordf(float(m3->image_point->v), int(m3->image_point->scale), 0);
            }
        }

        // so now go through and compute support for each point
        int support[actual_ransac_iterations];
        memset( &support, 0, sizeof(int)*actual_ransac_iterations );
        {
            // PROFILE PROFILE PROFILE PROFILE
            // profile results say that 99% of time is spent in this block
            // PROFILE PROFILE PROFILE PROFILE

            //PROFILE_THIS_BLOCK("estimate");
            affinity A;// = new affinity();

            for ( int i=0; i<actual_ransac_iterations; i++ )
            {
                /*float *nums = &(transformed_points[4*3*i]);
                A.estimate( nums[0], nums[1], nums[2], nums[3],
                           nums[4], nums[5], nums[6], nums[7],
                           nums[8], nums[9], nums[10], nums[11] );*/

                //A.estimate( &(transformed_points[4*3*i]) );
                A.estimate( transformed_points + 4*3*i );

                support[i] = detector->compute_support_for_affine_transformation_readonly(&A);
                /*if ( support[i] > detector->ransac_stop_support )
                    break;*/
            }
        }

        // find best support
        int best_support = -1;
        int best_index = 0;
        {
            //PROFILE_THIS_BLOCK("finalize");
            for ( int i=0; i<actual_ransac_iterations; i++ )
            {
                if ( support[i] > best_support )
                {
                    best_support = support[i];
                    best_index = i;
                }
            }
        }

        // re-estimate for the output
        if ( best_support > detector->best_support_thresh )
        {
            data->A.estimate( transformed_points + 4*3*best_index );
            data->A_support = best_support;
        }
        else
            data->A_support = -1;

        //printf("thread %i finished, best support %i\n", data->thread_id, best_support );

        // tell the barrier
        data->barrier->Wait();

    }

    pthread_exit(0);
}

void planar_object_recognizer::construct_match_lut()
{
    if ( do_proportional_match_lut )
    {
        // total scores
        float total_score = 0;
        for ( int i=0; i<match_number; i++ )
        {
            total_score += matches[i].score*matches[i].score;
        }
        int table_pos = 0;
        float score_remaining = total_score;
        float score_per_step = total_score/MATCH_LOOKUP_TABLE_SIZE;
        float steps_per_score = 1.0f/score_per_step;
        // normalise
        for ( int i=0; i<match_number; i++ )
        {
            float this_score = matches[i].score*matches[i].score;
            int end = table_pos+this_score*steps_per_score+1;
            //printf("%i: %f -> %i entries (end %i)\n", i, this_score, end-table_pos, end );
            for ( ; table_pos<end && table_pos < MATCH_LOOKUP_TABLE_SIZE; table_pos++ )
            {
                match_index_lookup[table_pos] = i;
            }
            if ( table_pos >= MATCH_LOOKUP_TABLE_SIZE )
                break;
        }
        assert( match_number ==0 || table_pos == MATCH_LOOKUP_TABLE_SIZE );
    }
    else
    {
        float curr_pos = 0;
        float step = float(MATCH_LOOKUP_TABLE_SIZE)/match_number;
        printf("step: %f\n", step );
        for ( int i=0; i<match_number; i++)
        {
            float next_start_pos = curr_pos+step;
            printf("%i: %i->%i\n", i, (int)curr_pos, (int)next_start_pos );
            for ( int j=curr_pos; j<next_start_pos && j<MATCH_LOOKUP_TABLE_SIZE; j++ )
                match_index_lookup[j] = i;
            curr_pos = next_start_pos;
        }
        //assert( match_number==0 || int(curr_pos) == MATCH_LOOKUP_TABLE_SIZE-1 );
    }
}

bool planar_object_recognizer::estimate_affine_transformation_mt(void)
{
    PROFILE_THIS_FUNCTION();

    // make threads
    int num_threads = 8;
    if ( affine_thread_data.size() != num_threads )
    {
        assert(affine_thread_data.size() == 0);

        // construct barrier
        shared_barrier = new FBarrier( num_threads+1 );

        // make threads joinable
        pthread_attr_t thread_attr;
        pthread_attr_init(&thread_attr);
        pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);
        printf("creating %i estimate affine transformation threads\n", num_threads);
        for ( int i=0; i<num_threads; i++ )
        {
            // setup data
            EstimateAffineThreadData* thread_data = new EstimateAffineThreadData();
            thread_data->thread_id = i;
            thread_data->detector = this;
            thread_data->barrier = shared_barrier;
            thread_data->should_stop = false;
            // start the thread
            pthread_create( &thread_data->thread, &thread_attr, estimate_affine_transformation_thread_func, (void*)thread_data );

            // store
            affine_thread_data.push_back( thread_data );
        }
        pthread_attr_destroy(&thread_attr);
    }

    // dump out three_random_corr input data
    /*printf("dumping keypoint matches:");
    for ( int i=0; i<match_number; i++ )
    {
        if ( i%10 == 0 )
            printf("\n%i: ", i);
        printf("%f ", matches[i].score);
    }
    printf("\n");*/
    PROFILE_SECTION_PUSH("match lut");
    // construct match lookup table
    construct_match_lut();
    PROFILE_SECTION_POP();

    // tell threads to run
    for ( int i=0; i<num_threads; i++ )
    {
        affine_thread_data[i]->A_support = -1;
        affine_thread_data[i]->num_ransac_iterations = max_ransac_iterations/num_threads;
        affine_thread_data[i]->start_signal.Signal();
    }

    // now wait for threads to finish
    shared_barrier->Wait();

    // and get best support
    affinity* best_A = NULL;
    int best_support = -1;
    int best_thread = -1;
    for ( int i=0; i<num_threads; i++ )
    {
        // read data back from the thread
        if ( affine_thread_data[i]->A_support > best_support)
        {
            best_A = &affine_thread_data[i]->A;
            best_support = affine_thread_data[i]->A_support;
            best_thread = i;
        }
    }

    /*if ( best_thread > 0 )
        printf("best thread was %i with %i support\n", affine_thread_data[best_thread]->thread_id, affine_thread_data[best_thread]->A_support );
    else
        printf("no best thread\n");*/

    // found?
    if ( best_support > best_support_thresh )
    {
        cvmCopy( best_A, affine_motion );
        int support = compute_support_for_affine_transformation(affine_motion);
        assert( support == best_support );

        return true;
    }
    else
        return false;

}


bool planar_object_recognizer::estimate_affine_transformation_unrolled(void)
{
    PROFILE_THIS_FUNCTION();

    // construct match lookup table
    construct_match_lut();

    // first, unroll everything

    // construct random correspondencies
    int randoms[3*max_ransac_iterations];
    int actual_ransac_iterations = max_ransac_iterations;
    {
        //PROFILE_THIS_BLOCK("three random");
        bool three_random = true;
        int i;
        for ( i=0; i < max_ransac_iterations && three_random; i++ )
        {
            // create 3 random correspondencies, bail if three_random_corr returns false
            three_random &= three_random_correspondences( &randoms[3*i], &randoms[3*i+1], &randoms[3*i+2] );
        }
        // in case three_random_correpsondencies fails early, we store how many we actually have
        actual_ransac_iterations = i;
    }
    //printf("%i actual iterations\n", actual_ransac_iterations );


    // transform points
    float transformed_points[4*3*actual_ransac_iterations];
    {
        //PROFILE_THIS_BLOCK( "transform" );
        for ( int i=0; i<actual_ransac_iterations; i++ )
        {
            image_object_point_match *m1 = matches + randoms[3*i+0];
            image_object_point_match *m2 = matches + randoms[3*i+1];
            image_object_point_match *m3 = matches + randoms[3*i+2];
            /*printf("%i: %x %x %x: ", i, m1, m2, m3 );
            printf(" %x ", m1->object_point );
            printf(" %x ", m2->object_point );
            printf(" %x ", m3->object_point );
            printf("\n");*/

            transformed_points[4*3*i+0 ] = PyrImage::convCoordf(float(m1->object_point->M[0]), int(m1->object_point->scale), 0);
            transformed_points[4*3*i+1 ] = PyrImage::convCoordf(float(m1->object_point->M[1]), int(m1->object_point->scale), 0);
            transformed_points[4*3*i+2 ] = PyrImage::convCoordf(float(m1->image_point->u), int(m1->image_point->scale), 0);
            transformed_points[4*3*i+3 ] = PyrImage::convCoordf(float(m1->image_point->v), int(m1->image_point->scale), 0);

            transformed_points[4*3*i+4 ] = PyrImage::convCoordf(float(m2->object_point->M[0]), int(m2->object_point->scale), 0);
            transformed_points[4*3*i+5 ] = PyrImage::convCoordf(float(m2->object_point->M[1]), int(m2->object_point->scale), 0);
            transformed_points[4*3*i+6 ] = PyrImage::convCoordf(float(m2->image_point->u), int(m2->image_point->scale), 0);
            transformed_points[4*3*i+7 ] = PyrImage::convCoordf(float(m2->image_point->v), int(m2->image_point->scale), 0);

            transformed_points[4*3*i+8 ] = PyrImage::convCoordf(float(m3->object_point->M[0]), int(m3->object_point->scale), 0);
            transformed_points[4*3*i+9 ] = PyrImage::convCoordf(float(m3->object_point->M[1]), int(m3->object_point->scale), 0);
            transformed_points[4*3*i+10] = PyrImage::convCoordf(float(m3->image_point->u), int(m3->image_point->scale), 0);
            transformed_points[4*3*i+11] = PyrImage::convCoordf(float(m3->image_point->v), int(m3->image_point->scale), 0);
        }
    }

    // so now go through and compute support for each point
    int support[actual_ransac_iterations];
    {
        //PROFILE_THIS_BLOCK("estimate");
        affinity A;// = new affinity();

        for ( int i=0; i<actual_ransac_iterations; i++ )
        {
            /*float *nums = &(transformed_points[4*3*i]);
            A.estimate( nums[0], nums[1], nums[2], nums[3],
                       nums[4], nums[5], nums[6], nums[7],
                       nums[8], nums[9], nums[10], nums[11] );*/

            //A.estimate( &(transformed_points[4*3*i]) );
            A.estimate( transformed_points + 4*3*i );

            support[i] = compute_support_for_affine_transformation(&A);
        }
    }

    // find best support
    int best_support = -1;
    int best_index = 0;
    {
        //PROFILE_THIS_BLOCK("finalize");
        for ( int i=0; i<actual_ransac_iterations; i++ )
        {
            if ( support[i] > best_support )
            {
                best_support = support[i];
                best_index = i;
            }
        }
    }

    // store average
    avg_ransac_iterations = (avg_ransac_iterations*7 + actual_ransac_iterations)/8;


    // finally
    if ( best_support > best_support_thresh )
    {
        //PROFILE_THIS_BLOCK( "return result " );
        affine_motion->estimate( transformed_points + 4*3*best_index );
        //affine_motion->estimate( &(transformed_points[4*3*best_index]) );
        /*float *nums = &(transformed_points[4*3*best_index]);
        affine_motion->estimate( nums[0], nums[1], nums[2], nums[3],
                   nums[4], nums[5], nums[6], nums[7],
                   nums[8], nums[9], nums[10], nums[11] );*/

        int support = compute_support_for_affine_transformation(affine_motion);
        assert( best_support == support );

        return true;
    }
    else
        return false;


}

bool planar_object_recognizer::estimate_affine_transformation(void)
{
    PROFILE_THIS_FUNCTION();

  affinity * A = new affinity();
  int iteration = 0;

  int best_support = -1;
  //printf("match number %i\n", match_number);

    // construct match lookup table
    construct_match_lut();

  do
  {
      PROFILE_THIS_BLOCK( "do loop" );
    int n1, n2, n3;

    iteration++;

    PROFILE_SECTION_PUSH( "three random" );
    bool three_random = three_random_correspondences(&n1, &n2, &n3);
    PROFILE_SECTION_POP();
    // Choose three correspondences randomly
    if (!three_random )
    {
      //cerr << "No three random correspondences found." << endl;
      break;
    }

    image_object_point_match * m1 = matches + n1;
    image_object_point_match * m2 = matches + n2;
    image_object_point_match * m3 = matches + n3;

    // Estimation of A (affine matrix) from the 3 correspondences:
    PROFILE_SECTION_PUSH( "A->estimate" );
    A->estimate
      (
        PyrImage::convCoordf(float(m1->object_point->M[0]), int(m1->object_point->scale), 0),
        PyrImage::convCoordf(float(m1->object_point->M[1]), int(m1->object_point->scale), 0),

        PyrImage::convCoordf(float(m1->image_point->u), int(m1->image_point->scale), 0),
        PyrImage::convCoordf(float(m1->image_point->v), int(m1->image_point->scale), 0),

        PyrImage::convCoordf(float(m2->object_point->M[0]), int(m2->object_point->scale), 0),
        PyrImage::convCoordf(float(m2->object_point->M[1]), int(m2->object_point->scale), 0),

        PyrImage::convCoordf(float(m2->image_point->u), int(m2->image_point->scale), 0),
        PyrImage::convCoordf(float(m2->image_point->v), int(m2->image_point->scale), 0),

        PyrImage::convCoordf(float(m3->object_point->M[0]), int(m3->object_point->scale), 0),
        PyrImage::convCoordf(float(m3->object_point->M[1]), int(m3->object_point->scale), 0),

        PyrImage::convCoordf(float(m3->image_point->u), int(m3->image_point->scale), 0),
        PyrImage::convCoordf(float(m3->image_point->v), int(m3->image_point->scale), 0)
      );
    PROFILE_SECTION_POP();

    PROFILE_SECTION_PUSH( "compute support for A" );
    int support = compute_support_for_affine_transformation(A);
    PROFILE_SECTION_POP();

    if (support > best_support)
    {
      best_support = support;

      cvmCopy(A, affine_motion);
      if (best_support > ransac_stop_support)
        break;
    }
  } while(iteration < max_ransac_iterations);

  avg_ransac_iterations = (avg_ransac_iterations*7 + iteration)/8;

  delete A;

  //cout << "Found support of " << best_support << " after " << iteration << " iterations." << endl;


  PROFILE_SECTION_PUSH("compute final support");
  int support = compute_support_for_affine_transformation(affine_motion);
  PROFILE_SECTION_POP();

  return support > best_support_thresh;
}

bool planar_object_recognizer::estimate_homographic_transformation_linear_method(void)
{
  int inlier_number = 0;
  int i;

  for(i = 0; i < match_number; i++)
    if (matches[i].inlier)
      inlier_number++;

  if (inlier_number < 10)
  {
    object_is_detected = false;
    return false;
  }

  H->set_match_number(inlier_number);
  for(i = 0; i < match_number; i++)
    if (matches[i].inlier)
      H->add_match(
      PyrImage::convCoordf(float(matches[i].object_point->M[0]), int(matches[i].object_point->scale), 0),
      PyrImage::convCoordf(float(matches[i].object_point->M[1]), int(matches[i].object_point->scale), 0),
      PyrImage::convCoordf(matches[i].image_point->u, int(matches[i].image_point->scale), 0),
      PyrImage::convCoordf(matches[i].image_point->v, int(matches[i].image_point->scale), 0));

  H->estimate();

  return true;
}

static void homography_error(
                             const ls_minimizer2::flt_t * state,
                             const ls_minimizer2::flt_t *d, int dsize,
                             ls_minimizer2::flt_t *b,
                             ls_minimizer2::flt_t *J,
                             void ** /*user_data*/)
{
  double den = 1. / (state[6] * d[0] + state[7] * d[1] + 1.);

  b[0] = (state[0] * d[0] + state[1] * d[1] + state[2]) * den;
  b[1] = (state[3] * d[0] + state[4] * d[1] + state[5]) * den;

  if (J != 0)
  {
    J[0] = d[0] * den;
    J[1] = d[1] * den;
    J[2] =      den;
    J[3] = 0;
    J[4] = 0;
    J[5] = 0;
    J[6] = -b[0] * J[0];
    J[7] = -b[0] * J[1];

    J[8]  = 0;
    J[9]  = 0;
    J[10] = 0;
    J[11] = J[0];
    J[12] = J[1];
    J[13] = J[2];
    J[14] = -b[1] * J[11];
    J[15] = -b[1] * J[12];
  }
}

bool planar_object_recognizer::estimate_homographic_transformation_nonlinear_method(void)
{
	int inlier_number = 0;

	for(int i = 0; i < match_number; i++)
		if (matches[i].inlier)
			inlier_number++;

	if (inlier_number < 10)
	{
		object_is_detected = false;
		return false;
	}

	homography_estimator->set_verbose_level(0);

	homography_estimator->reset_observations();
	for(int i = 0; i < match_number; i++)
		if (matches[i].inlier) {
			ls_minimizer2::flt_t d[2] = {
				PyrImage::convCoordf(float(matches[i].object_point->M[0]), int(matches[i].object_point->scale), 0),
				PyrImage::convCoordf(float(matches[i].object_point->M[1]), int(matches[i].object_point->scale), 0)};
			ls_minimizer2::flt_t b[2] = {
				PyrImage::convCoordf(matches[i].image_point->u, int(matches[i].image_point->scale), 0),
				PyrImage::convCoordf(matches[i].image_point->v, int(matches[i].image_point->scale), 0)};
			homography_estimator->add_observation_2data_2measures(homography_error, d, b);
		}

	double initial_state[8] = {affine_motion->cvmGet(0, 0), affine_motion->cvmGet(0, 1), affine_motion->cvmGet(0, 2),
		affine_motion->cvmGet(1, 0), affine_motion->cvmGet(1, 1), affine_motion->cvmGet(1, 2),
		0, 0};
	homography_estimator->minimize_using_levenberg_marquardt_from(initial_state);

	double * state = homography_estimator->state;

	H->cvmSet(0, 0, state[0]); H->cvmSet(0, 1, state[1]); H->cvmSet(0, 2, state[2]);
	H->cvmSet(1, 0, state[3]); H->cvmSet(1, 1, state[4]); H->cvmSet(1, 2, state[5]);
	H->cvmSet(2, 0, state[6]); H->cvmSet(2, 1, state[7]); H->cvmSet(2, 2, 1.);

    //printf("estimate_homographic nonlinear: ");
	homography_estimator->reset_observations();
	for (int iter=0; iter<2; iter++) {
		inlier_number=0;
		for (int i=0; i<match_number; i++) {
			double tu = PyrImage::convCoordf(matches[i].image_point->u,	int(matches[i].image_point->scale),	0);
			double tv = PyrImage::convCoordf(matches[i].image_point->v,	int(matches[i].image_point->scale),	0);
			double mu = PyrImage::convCoordf(float(matches[i].object_point->M[0]), int(matches[i].object_point->scale), 0);
			double mv = PyrImage::convCoordf(float(matches[i].object_point->M[1]), int(matches[i].object_point->scale), 0);
			double hmu, hmv;
			H->transform_point(mu, mv, &hmu, &hmv);
			double du = hmu-tu;
			double dv = hmv-tv;

			if (sqrt(du*du+dv*dv)< non_linear_refine_threshold) {
			    //printf("y");
				matches[i].inlier=true;
				inlier_number++;
				if (iter==0) {
					ls_minimizer2::flt_t d[2] = { mu, mv };
					ls_minimizer2::flt_t b[2] = { tu, tv };
					homography_estimator->add_observation_2data_2measures(homography_error, d, b);
				}
			} else {
				matches[i].inlier=false;
				//printf("n");
			}
		}
		if (iter==0 && inlier_number>=10){
			homography_estimator->minimize_using_levenberg_marquardt_from(state);
			state = homography_estimator->state;

			H->cvmSet(0, 0, state[0]); H->cvmSet(0, 1, state[1]); H->cvmSet(0, 2, state[2]);
			H->cvmSet(1, 0, state[3]); H->cvmSet(1, 1, state[4]); H->cvmSet(1, 2, state[5]);
			H->cvmSet(2, 0, state[6]); H->cvmSet(2, 1, state[7]); H->cvmSet(2, 2, 1.);
		}
	}
	//printf("\n");
	if (inlier_number < 10){
		object_is_detected = false;
		return false;
	}

	homography_estimator->reset_observations();

	return true;
}

void planar_object_recognizer::save_patch_before_and_after_correction(IplImage * image_before_smoothing,
                                                                      int u, int v, int patch_size,
                                                                      image_class_example * pv,
                                                                      int point_index,
                                                                      int call_number)
{
  char name[1000];

  IplImage * p = mcvGetPatch(image_before_smoothing, u, v, patch_size, patch_size);
  sprintf(name, "patches/kp_%03d_%04d.bmp", point_index, call_number);
  printf("Saving image in %s\n", name);
  mcvSaveImage(name, p);
  cvReleaseImage(&p);

  sprintf(name, "patches/kp_%03d_%04d_corrected.bmp", point_index, call_number);
  mcvSaveImage(name, pv->preprocessed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

pair<object_keypoint, int> * planar_object_recognizer::search_for_existing_model_point(vector< pair<object_keypoint, int> > * tmp_model_points,
                                                                                       float cu, float cv, int scale)
{
  for(vector< pair<object_keypoint, int> >::iterator it = tmp_model_points->begin();
    it < tmp_model_points->end();
    it++)
    if (int(it->first.scale) == scale)
    {
      float dist2 =
        (float(it->first.M[0]) - cu) * (float(it->first.M[0]) - cu) +
        (float(it->first.M[1]) - cv) * (float(it->first.M[1]) - cv);

      if (dist2 < keypoint_distance_threshold * keypoint_distance_threshold)
        return &(*it);
    }

    return 0;
}

bool cmp_tmp_model_points(pair<object_keypoint, int> p1, pair<object_keypoint, int> p2)
{
  return p1.second > p2.second;
}

// Selection of the interest points from the image model
void planar_object_recognizer::detect_most_stable_model_points(int max_point_number_on_model,
                                                               int patch_size,
                                                               int view_nb, double min_view_rate,
                                                               LEARNPROGRESSION LearnProgress)
{
    put_ui_settings();
  cout << "Determining most stable points:" << endl;

  vector< pair<object_keypoint, int> > tmp_model_points;
  int K = 8;

  keypoint * model_points_2d = new keypoint[K * max_point_number_on_model];

  // First detection of interest points in the image model (frontal view):
  int old_tau = point_detector->get_tau();
  static const int NEW_TAU = 10;
  printf("setting temporary point detector tau (old %i) -> %i\n", old_tau, NEW_TAU );
  point_detector->set_tau(NEW_TAU);
  int model_point2d_number = point_detector->pyramidBlurDetect(new_images_generator.original_image, model_points_2d,
                                                               K * max_point_number_on_model);
  printf("found %i points in image\n", model_point2d_number);
  printf("restoring point detector tau -> %i\n", old_tau );
  point_detector->set_tau(old_tau);

  // Save an image of the interest points detected in the frontal view
  model_point_number = 0;
  model_points = new object_keypoint[K*max_point_number_on_model];
  for(int i = 0; i < model_point2d_number; i++)
  {
    keypoint * k = model_points_2d + i;
    if (new_images_generator.inside_roi( // Keep point only if it is on the target roi
                                        int(PyrImage::convCoordf(k->u, int(k->scale), 0)),
                                        int(PyrImage::convCoordf(k->v, int(k->scale), 0))))
    {
      model_points[model_point_number].M[0]  = k->u;
      model_points[model_point_number].M[1]  = k->v;
      model_points[model_point_number].M[2]  = 0;
      model_points[model_point_number].scale = k->scale;

      model_point_number++;

      if (model_point_number >= K*max_point_number_on_model)
        break;
    }
  }
  printf("found %i points in target ROI\n", model_point_number);

  save_image_of_model_points(patch_size, "initial_model_points.bmp");
  delete [] model_points;
  model_point_number = 0;

  // Create tmp_model_points (vector) from model_points_2d (array of keypoints)
  // tmp_model_points will be used afterward
  for(int i = 0; i < model_point2d_number; i++)
  {
    keypoint * k = model_points_2d + i;

    if (new_images_generator.inside_roi( // Keep point only if it is on the target roi
                                        int(PyrImage::convCoordf(k->u, int(k->scale), 0)),
                                        int(PyrImage::convCoordf(k->v, int(k->scale), 0))))
    {
      pair<object_keypoint, int> * mp = search_for_existing_model_point(&tmp_model_points, k->u, k->v, int(k->scale));

      if (mp != 0) // Agglomerate clustered keypoints into one single keypoint
      {
        double n = double(++mp->second);
        mp->first.M[0] = (mp->first.M[0]*(n-1) + k->u)/n;
        mp->first.M[1] = (mp->first.M[1]*(n-1) + k->v)/n;
      }
      else
      {
        object_keypoint op;
        op.M[0] = k->u;
        op.M[1] = k->v;
        op.M[2] = 0;
        op.scale = k->scale;
        tmp_model_points.push_back(pair<object_keypoint, int>(op, 1));
      }
    }
  }


  // Generate random views and run detection on each view to determine the most frequent interest points
  bool use_random_background = new_images_generator.use_random_background;
  new_images_generator.set_use_random_background(false);

  for(int j = 0; j < view_nb; j++)
  {
    if (LearnProgress!=0)
      LearnProgress(DETECT_MODEL_POINTS, j, view_nb);
    else
      cout << "Generating views: " << view_nb - j << "         \r" << flush;

    new_images_generator.generate_random_affine_transformation();
    new_images_generator.generate_object_view();

    int current_detected_point_number = point_detector->detect(&(new_images_generator.smoothed_generated_object_view->image),
                                                                 model_points_2d, K * max_point_number_on_model);

    for(int i = 0; i < current_detected_point_number; i++)
    {
      keypoint * k = model_points_2d + i;
      float nu, nv;
      new_images_generator.inverse_affine_transformation(PyrImage::convCoordf(k->u, int(k->scale), 0),
                                                         PyrImage::convCoordf(k->v, int(k->scale), 0),
                                                         nu, nv);

      nu = PyrImage::convCoordf(nu, 0, int(k->scale));
      nv = PyrImage::convCoordf(nv, 0, int(k->scale));

      keypoint kd;
      kd.u = nu;
      kd.v = nv;
      kd.scale = k->scale;
      if (new_images_generator.inside_roi( // Keep point only if it is on the target roi
                                          int(PyrImage::convCoordf(k->u, int(k->scale), 0)),
                                          int(PyrImage::convCoordf(k->v, int(k->scale), 0))))
      {
        pair<object_keypoint, int> * mp = search_for_existing_model_point(&tmp_model_points, nu, nv, int(k->scale));

        if (mp != 0)  // Agglomerate clustered keypoints into one single keypoint
        {
          // move the keypoint coordinates in the center of gravity of
          // all agglomerated keypoints.
          double n = double(++mp->second);
          mp->first.M[0] = (mp->first.M[0]*(n-1) + nu)/n;
          mp->first.M[1] = (mp->first.M[1]*(n-1) + nv)/n;
        }
        else
        {
          object_keypoint op;
          op.M[0] = nu;
          op.M[1] = nv;
          op.M[2] = 0;
          op.scale = k->scale;
          tmp_model_points.push_back(pair<object_keypoint, int>(op, 1));
        }
      }
    }
  }

  new_images_generator.set_use_random_background(use_random_background);

  sort(tmp_model_points.begin(), tmp_model_points.end(), cmp_tmp_model_points);

  int min_views = int(min_view_rate * (double)view_nb);
  int i;
  model_points = new object_keypoint[max_point_number_on_model];
  vector< pair<object_keypoint, int> >::iterator it;

  for( it = tmp_model_points.begin(), i = 0;
      (it < tmp_model_points.end()) && (i < max_point_number_on_model) && (it->second >= min_views);
       it++, i++)
  {
    model_points[i].M[0] = it->first.M[0];
    model_points[i].M[1] = it->first.M[1];
    model_points[i].M[2] = it->first.M[2];
    model_points[i].scale = it->first.scale;
    model_points[i].class_index = i;

    if (i == 0)
      cout << "Point " << i << " detected " << it->second << " times." << endl;
  }
  cout << "... Point " << i << " detected " << it->second << " times ("
       << view_nb << " generated views, "
       << min_view_rate*100 << "% minimum rate)." << endl;
  cout << i << " points in total." << endl;
  model_point_number = i;

  new_images_generator.set_object_keypoints(model_points, model_point_number);

  delete [] model_points_2d;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// VISUALIZATION: The following functions are useful for visualization only !!!

void planar_object_recognizer::save_image_of_model_points(int patch_size, const char * filename)
{
  IplImage* model_image = mcvGrayToColor(new_images_generator.original_image);

  for(int i = 0; i < model_point_number; i++)
    cvCircle(model_image,
             cvPoint((int)PyrImage::convCoordf(float(model_points[i].M[0]), int(model_points[i].scale), 0), // x
                     (int)PyrImage::convCoordf(float(model_points[i].M[1]), int(model_points[i].scale), 0)), // y
                     (int)PyrImage::convCoordf(patch_size/8.f, int(model_points[i].scale), 0), // radius
             mcvRainbowColor(int(model_points[i].scale)), // color
             1);

  if (filename == 0)
    mcvSaveImage("model_points.bmp", model_image);
  else
    mcvSaveImage(filename, model_image);

  cvReleaseImage(&model_image);
}

IplImage * planar_object_recognizer::create_result_image(IplImage * input_image,
                                                         bool p_draw_points,
                                                         bool p_draw_matches,
                                                         bool p_draw_object,
                                                         bool p_draw_model_image,
                                                         int line_width)
{
  concat_model_and_input_images(input_image, p_draw_model_image);

  if (p_draw_points) draw_points(line_width);
  if (p_draw_model_image)
  {
    if (p_draw_matches && object_is_detected) draw_matches(line_width);
    if (p_draw_object && object_is_detected) draw_model();
  }
  else
    if (p_draw_object && object_is_detected) draw_model();

  IplImage *im= model_and_input_images;
  model_and_input_images=0;
  return im;
}

void planar_object_recognizer::draw_model(void)
{
  CvPoint C1 = cvPoint(int(u_input_image + detected_u_corner1), int(v_input_image + detected_v_corner1));
  CvPoint C2 = cvPoint(int(u_input_image + detected_u_corner2), int(v_input_image + detected_v_corner2));
  CvPoint C3 = cvPoint(int(u_input_image + detected_u_corner3), int(v_input_image + detected_v_corner3));
  CvPoint C4 = cvPoint(int(u_input_image + detected_u_corner4), int(v_input_image + detected_v_corner4));
  CvScalar col;

  col = cvScalar(0, 0, 0);
  cvLine(model_and_input_images, C1, C2, col, 6);
  cvLine(model_and_input_images, C2, C3, col, 6);
  cvLine(model_and_input_images, C3, C4, col, 6);
  cvLine(model_and_input_images, C4, C1, col, 6);

  col = cvScalar(255, 255, 255);
  cvLine(model_and_input_images, C1, C2, col, 2);
  cvLine(model_and_input_images, C2, C3, col, 2);
  cvLine(model_and_input_images, C3, C4, col, 2);
  cvLine(model_and_input_images, C4, C1, col, 2);
}

void planar_object_recognizer::save_one_image_per_match_input_to_model(IplImage * input_image, const char * matches_dir)
{
  int patch_size = forest->image_width;

  for(int i = 0; i < detected_point_number; i++)
  {
    concat_model_and_input_images(input_image);
    draw_input_image_points();

    image_class_example * pv = &(detected_point_views[i]);
    float u = pv->point2d->u, v = pv->point2d->v;
    int s = int(pv->point2d->scale);

    if (u > (patch_size/2) && u < object_input_view->image[s]->width - (patch_size/2) &&
        v > (patch_size/2) && v < object_input_view->image[s]->height - (patch_size/2))
    {
      CvPoint ip = cvPoint(u_input_image + int(PyrImage::convCoordf(pv->point2d->u, int(pv->point2d->scale),0)),
                           v_input_image + int(PyrImage::convCoordf(pv->point2d->v, int(pv->point2d->scale),0)));

      cvCircle(model_and_input_images, ip,
               int(PyrImage::convCoordf(patch_size/2.f, int(pv->point2d->scale),0)), cvScalar(255, 255, 255), 1);

      float best_P = match_probabilities[i][0];
      int index_best_P = 0;
      for(int j = 0; j < model_point_number; j++)
      {
        if (match_probabilities[i][j] > 0.05)
        {
          object_keypoint * M = &(model_points[j]);
          int level = MIN(255, int(match_probabilities[i][j] / 0.2 * 255));
          CvScalar col = cvScalar(level, level, level);
          CvPoint mp = cvPoint(int(PyrImage::convCoordf(float(M->M[0]), int(M->scale), 0)),
                               int(PyrImage::convCoordf(float(M->M[1]), int(M->scale), 0)));

          cvCircle(model_and_input_images, mp, (int)PyrImage::convCoordf(patch_size/2.f, int(M->scale), 0), col, 1);


          cvLine(model_and_input_images, ip, mp, col, 1);
        }
        if (best_P < match_probabilities[i][j])
        {
          best_P = match_probabilities[i][j];
          index_best_P = j;
        }
      }

      {
        object_keypoint * M = &(model_points[index_best_P]);
        int level = MIN(255, int(match_probabilities[i][index_best_P] / 0.2 * 255));
        CvScalar col = cvScalar(level, level, level);
        CvPoint mp = cvPoint(int(PyrImage::convCoordf(float(M->M[0]), int(M->scale), 0)),
                             int(PyrImage::convCoordf(float(M->M[1]), int(M->scale), 0)));

        cvCircle(model_and_input_images, mp, int(PyrImage::convCoordf(patch_size/2.f, int(M->scale),0)),col,1);

        cvLine(model_and_input_images, ip, mp, col, 1);
      }
    }
    {
      char filename[10000];
      sprintf(filename, "%s/match%04d.bmp", matches_dir, i);
      mcvSaveImage(filename, model_and_input_images);
    }
  }
}

void planar_object_recognizer::save_one_image_per_match_model_to_input(IplImage * input_image, const char * matches_dir)
{
  int patch_size = forest->image_width;

  for(int j = 0; j < model_point_number; j++)
  {
    concat_model_and_input_images(input_image);
    //  draw_input_image_points();

    object_keypoint * M = &(model_points[j]);
    CvPoint mp = cvPoint(int(PyrImage::convCoordf(float(M->M[0]), int(M->scale), 0)),
                         int(PyrImage::convCoordf(float(M->M[1]), int(M->scale), 0)));
    cvCircle(model_and_input_images, mp,
             int(PyrImage::convCoordf(patch_size/2.f, int(M->scale),0)), cvScalar(0, 0, 0), 3);
    cvCircle(model_and_input_images, mp,
             int(PyrImage::convCoordf(patch_size/2.f, int(M->scale),0)), cvScalar(255, 255, 255), 1);

    float best_P = match_probabilities[0][j];
    int index_best_P = 0;
    for(int i = 0; i < detected_point_number; i++)
    {
      image_class_example * pv = &(detected_point_views[i]);
      float u = pv->point2d->u, v = pv->point2d->v;
      int s = int(pv->point2d->scale);

      if (u > (patch_size / 2) && u < object_input_view->image[s]->width - (patch_size / 2) &&
          v > (patch_size / 2) && v < object_input_view->image[s]->height - (patch_size / 2))
      {
        if (best_P < match_probabilities[i][j])
        {
          best_P = match_probabilities[i][j];
          index_best_P = i;
        }
      }
    }

    {
      image_class_example * pv = &(detected_point_views[index_best_P]);
      float u = pv->point2d->u, v = pv->point2d->v;
      int s = int(pv->point2d->scale);

      CvPoint ip = cvPoint(u_input_image + int(PyrImage::convCoordf(u,s,0)),
                           v_input_image + int(PyrImage::convCoordf(v,s,0)));

      cvCircle(model_and_input_images, ip,
        int(PyrImage::convCoordf(patch_size/2.f,int(pv->point2d->scale),0)),
        cvScalar(0, 0, 0), 3);
      cvLine(model_and_input_images, ip, mp, cvScalar(0, 0, 0), 3);

      cvCircle(model_and_input_images, ip,
              int(PyrImage::convCoordf(patch_size/2.f,int(pv->point2d->scale),0)),
              cvScalar(255, 255, 255), 2);
      cvLine(model_and_input_images, ip, mp, cvScalar(255, 255, 255), 2);
    }

    char image_name[1000];
    sprintf(image_name, "%s/match%04d.bmp", matches_dir, j);
    mcvSaveImage(image_name, model_and_input_images);
  }
}


void planar_object_recognizer::save_one_image_per_match(IplImage * input_image, const char * matches_dir)
{
  int patch_size = forest->image_width;

  for(int i = 0; i < match_number; i++)
  {
    image_object_point_match * match = &(matches[i]);

    if (!match->inlier)
      continue;

    concat_model_and_input_images(input_image);

    CvPoint M = cvPoint(int(PyrImage::convCoordf(float(match->object_point->M[0]), int(match->object_point->scale), 0)),
                        int(PyrImage::convCoordf(float(match->object_point->M[1]), int(match->object_point->scale), 0)));

    cvCircle(model_and_input_images,
             M, int(PyrImage::convCoordf(patch_size/2.f, int(match->object_point->scale), 0)), cvScalar(0, 0, 0), 3);
    cvCircle(model_and_input_images,
             M, int(PyrImage::convCoordf(patch_size/2.f, int(match->object_point->scale), 0)), cvScalar(255, 255, 255), 1);

    CvPoint m = cvPoint(u_input_image + (int)PyrImage::convCoordf(match->image_point->u, int(match->object_point->scale), 0),
                        v_input_image + (int)PyrImage::convCoordf(match->image_point->v, int(match->object_point->scale), 0));

    cvCircle(model_and_input_images,
             m, int(PyrImage::convCoordf(patch_size/2.f, int(match->object_point->scale), 0)), cvScalar(0, 0, 0), 3);
    cvCircle(model_and_input_images,
             m, int(PyrImage::convCoordf(patch_size/2.f, int(match->object_point->scale), 0)), cvScalar(255, 255, 255), 1);

    char image_name[1000];
    sprintf(image_name, "%s/match%04d.bmp", matches_dir, i);
    mcvSaveImage(image_name, model_and_input_images);
  }
}


void planar_object_recognizer::concat_model_and_input_images(IplImage * input_image, bool draw_model_image)
{
  if (model_and_input_images != 0)
    cvReleaseImage(&model_and_input_images);

  if (draw_model_image)
  {
    model_and_input_images =
      cvCreateImage(cvSize(max(new_images_generator.original_image->width, input_image->width),
      new_images_generator.original_image->height + input_image->height + 10), IPL_DEPTH_8U, 3);

    cvSet(model_and_input_images, cvScalar(128, 128, 128));

    x0 = 0;
    y0 = 0;
    mcvPut(model_and_input_images, new_images_generator.original_image, x0, y0);

    u_input_image = 0;
    v_input_image = new_images_generator.original_image->height + 10;
    mcvPut(model_and_input_images, input_image, u_input_image, v_input_image);
  }
  else
  {
    model_and_input_images = cvCloneImage(input_image);

    x0 = 0;
    y0 = 0;

    u_input_image = 0;
    v_input_image = 0;
  }
}

void planar_object_recognizer::draw_points(int line_width)
{
  draw_model_points(line_width);
  draw_input_image_points(line_width);
}

void planar_object_recognizer::draw_model_points(int line_width)
{
  int patch_size = forest->image_width;

  for(int i = 0; i < model_point_number; i++)
    cvCircle(model_and_input_images,
    cvPoint(x0 + (int)PyrImage::convCoordf(float(model_points[i].M[0]), int(model_points[i].scale), 0),
            y0 + (int)PyrImage::convCoordf(float(model_points[i].M[1]), int(model_points[i].scale), 0)),
           (int)PyrImage::convCoordf(patch_size/2.f, int(model_points[i].scale), 0), CV_RGB(0, 0, 255), line_width);
}

void planar_object_recognizer::draw_input_image_points(int line_width)
{
  int patch_size = forest->image_width;

  for(int i = 0; i < detected_point_number; i++)
    cvCircle(model_and_input_images,
             cvPoint(u_input_image + (int)PyrImage::convCoordf(detected_points[i].u, int(detected_points[i].scale), 0),
                     v_input_image + (int)PyrImage::convCoordf(detected_points[i].v, int(detected_points[i].scale), 0)),
             (int)PyrImage::convCoordf(patch_size/2.f, int(detected_points[i].scale), 0),
             mcvRainbowColor(int(detected_points[i].scale)), line_width);
}

void planar_object_recognizer::draw_matches(int line_width)
{
  for(int i = 0; i < match_number; i++)
  {
    image_object_point_match * match = &(matches[i]);

    if (match->inlier)
    {
      CvScalar col = cvScalar(255, 255, 255);

      cvLine(model_and_input_images,
        cvPoint(int(PyrImage::convCoordf(float(match->object_point->M[0]), int(match->object_point->scale), 0)),
                int(PyrImage::convCoordf(float(match->object_point->M[1]), int(match->object_point->scale), 0))),
                 cvPoint(u_input_image + int(PyrImage::convCoordf(match->image_point->u, int(match->image_point->scale), 0)),
                         v_input_image + int(PyrImage::convCoordf(match->image_point->v, int(match->image_point->scale), 0))),
        col, line_width);
    }
  }
}

void planar_object_recognizer::draw_inlier_matches(int line_width)
{
  for(int i = 0; i < match_number; i++)
  {
    image_object_point_match * match = &(matches[i]);

    if (match->inlier)
    {
      cvLine(model_and_input_images,
             cvPoint( u_input_image+int(match->image_point->u), v_input_image+int(match->image_point->v) ),
             cvPoint((int)match->object_point->M[0], (int)match->object_point->M[1]),
             CV_RGB(255,0,0), line_width);
    }
  }
}

/*! Ensures that allocated resources are ready to handle an image of the given
 * size.
 */
void planar_object_recognizer::check_target_size(IplImage *image)
{
	int w = image->width;
	int h = image->height;
	int nbLev = new_images_generator.level_number;

	if (object_input_view->image[0]->width == w &&
			object_input_view->image[0]->height ==h)
		// Nothing to do: allocated resources have the right size
		return;

	//int u[4] = {-1, -1, -1, -1};
	//int v[4] = {-1, -1, -1, -1};

	if (new_images_generator.orientation_corrector != 0)
		delete new_images_generator.orientation_corrector;
	new_images_generator.orientation_corrector = new keypoint_orientation_corrector(
			w,h, new_images_generator.patch_size, nbLev);

	delete object_input_view;
	object_input_view = new object_view(w,h,nbLev);
	int yape_radius = point_detector->get_radius();
	delete point_detector;
	point_detector = new pyr_yape(w,h,nbLev);
	point_detector->set_radius(yape_radius);
}

void planar_object_recognizer::get_ui_settings()
{
    ransac_dist_threshold       = ransac_dist_threshold_ui;
    ransac_stop_support         = ransac_stop_support_ui;
    max_ransac_iterations       = max_ransac_iterations_ui;
    non_linear_refine_threshold = non_linear_refine_threshold_ui;
    match_score_threshold       = match_score_threshold_ui;
    best_support_thresh         = best_support_thresh_ui;
    point_detector_tau          = point_detector_tau_ui;

}
void planar_object_recognizer::put_ui_settings()
{
    ransac_dist_threshold_ui    = ransac_dist_threshold;
    ransac_stop_support_ui      = ransac_stop_support;
    max_ransac_iterations_ui    = max_ransac_iterations;
    non_linear_refine_threshold_ui = non_linear_refine_threshold;
    match_score_threshold_ui    = match_score_threshold;
    best_support_thresh_ui      = best_support_thresh;
    point_detector_tau_ui       = point_detector_tau;
}

void planar_object_recognizer::dump()
{
    printf("dumping %3i model points\n", model_point_number );
    for ( int i=0; i<model_point_number; i++)
    {
        printf(" %3i: ", i );
        object_keypoint& op = model_points[i];
        op.dump();
    }
    printf("dumping forest\n");
    forest->dump();
    printf("done");
}

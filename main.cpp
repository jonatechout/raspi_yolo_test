#include <darknet.h>
#include <nnpack.h>
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sys/time.h>
#include <raspicam/raspicam_cv.h>

using namespace cv;

void convertCvImageToDnImage(const Mat& mat, image* out_img)
{
    for(int y = 0; y < out_img->h; ++y){
      for(int x = 0; x < out_img->w; ++x){
        for(int k= 0; k < out_img->c; ++k){
          out_img->data[k * out_img->h * out_img->w + y * out_img->w + x]
            = static_cast<float>(mat.data[y * mat.step + x * mat.elemSize() + k]) / 255.0;
        }
      }
    }
}

void convertDnImageToCvImage(const image& dnetImg, Mat* out_mat)
{
  for(int y = 0; y < dnetImg.h; ++y){
    for(int x = 0; x < dnetImg.w; ++x){
      for(int k= 0; k < dnetImg.c; ++k){
        out_mat->data[y * out_mat->step + x * out_mat->elemSize() + k]
          =  static_cast<unsigned char>(dnetImg.data[k * dnetImg.h * dnetImg.w + y * dnetImg.w + x] * 255);
      }
    }
  }
}

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    printf("4 arguments are necessary.\n");
    printf("%s <data config path> <network config path> <network weights path>\n", argv[0]);
    return -1;
  }

  const unsigned int IMAGE_WIDTH = 320;
  const unsigned int IMAGE_HEIGHT = 240;

  raspicam::RaspiCam_Cv camera;
  camera.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
  camera.set( CV_CAP_PROP_FRAME_WIDTH, IMAGE_WIDTH );
  camera.set( CV_CAP_PROP_FRAME_HEIGHT, IMAGE_HEIGHT );

  if (!camera.open())
  {
    printf("camera open error\n");
    return -1;
  }

  char *datacfg = argv[1];
  char *cfgfile = argv[2];
  char *weightfile = argv[3];
  float thresh = 0.24;
  float hier_thresh = 0.5;

  list *options = read_data_cfg(datacfg);
  char *name_list = option_find_str(options, const_cast<char*>("names"), const_cast<char*>("data/names.list"));
  char **names = get_labels(name_list);

  image **alphabet = load_alphabet();
  network *net = load_network(cfgfile, weightfile, 0);
  set_batch_network(net, 1);
  srand(2222222);

  float nms=.3;
  nnp_initialize();
  net->threadpool = pthreadpool_create(0);

  namedWindow("prediction", CV_WINDOW_AUTOSIZE);
  image dnetImg = make_image(IMAGE_WIDTH, IMAGE_HEIGHT, 3);

  layer l = net->layers[net->n-1];
  box *boxes = static_cast<box*>(calloc(l.w*l.h*l.n, sizeof(box)));
  float **probs = static_cast<float**>(calloc(l.w*l.h*l.n, sizeof(float *)));

  for(int j = 0; j < l.w*l.h*l.n; ++j) {
    probs[j] = static_cast<float*>(calloc(l.classes + 1, sizeof(float *)));
  }

  printf("ready\n");

  while(1){
    Mat mat;
    camera.grab();
    camera.retrieve(mat);

    convertCvImageToDnImage(mat, &dnetImg);

    image sized = letterbox_image_thread(dnetImg, net->w, net->h, net->threadpool);
    float *X = sized.data;

    struct timeval start, stop;
    gettimeofday(&start, 0);

    network_predict(net, X);

    gettimeofday(&stop, 0);
    printf("Predicted in %ld ms.\n", (stop.tv_sec * 1000 + stop.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000));

    get_region_boxes(l, dnetImg.w, dnetImg.h, net->w, net->h, thresh, probs, boxes, NULL, 0, 0, hier_thresh, 1);

    if (nms) do_nms_sort(boxes, probs, l.w*l.h*l.n, l.classes, nms);
    draw_detections(dnetImg, l.w*l.h*l.n, thresh, boxes, probs, NULL, names, alphabet, l.classes);

    Mat disp(dnetImg.h, dnetImg.w, CV_8UC3);
    convertDnImageToCvImage(dnetImg, &disp);
    imshow("prediction", disp);
    if (waitKey(1) >= 0) break;

    free_image(sized);
  }

  free_image(dnetImg);
  free(boxes);
  free_ptrs((void **)probs, l.w*l.h*l.n);

  pthreadpool_destroy(net->threadpool);
  nnp_deinitialize();

  return 0;
}

// 
/*
    
*/

#include <dlib/opencv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
//#include <opencv2/core/types.hpp>

#include <dlib/dnn.h>
#include <dlib/gui_widgets.h>
#include <dlib/string.h>
#include <dlib/image_io.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/serialize.h>

//#include <my_global.h>
#include <mysql.h>

#include <json/json.h>
#include <chrono>

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
void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);        
}
	
int main(int argc, char** argv) try
{	
	if(argc <5)
	{
	  cout << "{error: ""Too few arguments - should be 5 min""}" << endl;
	  return -1;
	}
  
	bool debug_mode_flag = false;
	if (argc > 5) debug_mode_flag = true;
	 
	bool gui_mode_flag;
	if(atoi(argv[5]) == 0) gui_mode_flag = false;
	

	int id_person = atoi(argv[1]);
	int id_camera = atoi(argv[2]);
	int max_averaging_count = atoi(argv[3]);
	int max_blank_frames_count = 100;

	//Connecting to the MySQL database
	cout << "MySQL client version: " << mysql_get_client_info() << endl;
		
	MYSQL *connection = mysql_init(NULL);
	if(connection == NULL) 
	{
	  cout << "{error: ""Unable to create MySQL connection""}" << endl;
	  return -1;	  
	}
	
	//if (mysql_real_connect(connection, "localhost", "root", "i20cvpass", "video", 0, NULL, 0) == NULL)
	if (mysql_real_connect(connection, "195.211.7.218", "video", "Vej7Tuf7Up", "video", 53306, NULL, 0) == NULL)
	{
	  fprintf(stderr, "%s\n", mysql_error(connection));
	  mysql_close(connection);
	  return -1;
	}  

		
	// The first thing we are going to do is load all our models.  First, since we need to
	// find faces in the image we will need a face detector:
	frontal_face_detector detector = get_frontal_face_detector();

	// We will also use a face landmarking model to align faces to a standard pose:  (see face_landmark_detection_ex.cpp for an introduction)
	shape_predictor sp;
	dlib::deserialize("shape_predictor_5_face_landmarks.dat") >> sp;
	// And finally we load the DNN responsible for face recognition.
	anet_type net;
	dlib::deserialize("dlib_face_recognition_resnet_model_v1.dat") >> net;

	//cv::VideoCapture  cap(id_camera);	
	cv::VideoCapture  cap;	
	cv::VideoCapture* capture;
			
	/*
	if(argc > 5)
	{
	  cap = cv::VideoCapture(argv[6]);
	} 
	else */
	cap = cv::VideoCapture(id_camera);
	//cap.cv::VideoCapture::set(cv::CAP_PROP_FRAME_WIDTH, 320);
	//cap.cv::VideoCapture::set(cv::CAP_PROP_FRAME_HEIGHT, 240);

	
	cv::Mat in, frame;	

	int frame_count = 0;
	int same_count = 0;
	int blank_count = 0;

	// Display the raw image on the screen
	image_window win;
	image_window win_faces;

	matrix<float, 0, 1> res_descriptor;
	matrix<rgb_pixel> dlib_avatar;

	while (same_count < max_averaging_count && blank_count < max_blank_frames_count)
	{
		if (!cap.read(in))
		{
		  cout << "{error: ""Unable to get video stream""}" << endl;
		  return -1;
		}

		frame_count++;
		cv_image<bgr_pixel> img(in);
		
		// Run the face detector on the image captured from camera, and for each face extract a
		// copy that has been normalized to 150x150 pixels in size and appropriately rotated
		// and centered.
		std::vector<matrix<rgb_pixel>> faces;
		std::vector<full_object_detection> shapes;
		for (auto face : detector(img))
		{
			auto shape = sp(img, face);
			matrix<rgb_pixel> face_chip;
			extract_image_chip(img, get_face_chip_details(shape, 150, 0.25), face_chip);
			faces.push_back(move(face_chip));
			// Also put some boxes on the faces so we can see that the detector is finding them.
			shapes.push_back(shape);
			//win.add_overlay(face);
		}

		//need only ONE detected face
		if (faces.size() > 1  ) 
		{ 
			cout << "More than one face detected!! Clear counter" << endl;
			same_count = 0; 
			blank_count = 0;
			win.clear_overlay();
			win.set_image(img);
			continue;
		}

		//if no face detected on the frame
		if (faces.size() == 0)
		{
			cout << "No faces found in image! blank_count = " << blank_count << endl;
			win.clear_overlay();
			win.set_image(img);
			same_count = 0;
			blank_count++; 
			continue;
		}

		// This call asks the DNN to convert each face image in faces into a 128D vector.
		// In this 128D vector space, images from the same person will be close to each other
		// but vectors from different people will be far apart.  So we can use these vectors to
		// identify if a pair of images are from the same person or from different people.  		
		//double t = (double)GetTickCount();
		std::vector<matrix<float, 0, 1>> face_descriptors = net(faces);
		//t = ((double)GetTickCount() - t);
		//cout << "Faces found:" << faces.size() << " CNN time:" << t << "ms\t";

		if (frame_count > 1) 
		{		
			res_descriptor = (res_descriptor + face_descriptors[0]) / 2;
			if (length(res_descriptor - face_descriptors[0]) < 0.6)
			{
				//cout << "Descriptor averaged. Same face!!! \n";
				same_count++;
				blank_count = 0;
			}
			else
			{
				cout << "Error descriptor not same - keep averaging loop \n";
				same_count = 0;
				blank_count = 0;
			}
		}
		else
		{
			res_descriptor = face_descriptors[0];			
		}

		dlib_avatar = faces[0];
		
		// Finally, let's print one of the face descriptors to the screen.  
		//cout << "Face descriptor for " << frame_count << " frame face: \n" << trans(res_descriptor) << endl;

		if(gui_mode_flag)
		{
		  win_faces.set_title("Detected faces");
		  win_faces.set_image(tile_images(faces));				
		
		  win.clear_overlay();
		  win.set_image(img);
		  win.add_overlay(render_face_detections(shapes));
		}
	}
	
	if(blank_count == max_blank_frames_count)
	{
	  cout << "{error: ""Unable to find any face""} " << endl;
	  return -1;
	}
	
	
	//Saving last image chip as avatar
	//TODO: Выдёргивать чип из входного изображения используя box информцию,а не через dlib
	cv::Mat img_avatar = dlib::toMat(dlib_avatar);
	//opencv 2.4.9 валиться тут!!!
	cv::cvtColor(img_avatar, img_avatar, cv::COLOR_RGB2BGR);
	std::string path(argv[4]);
	std::string filename("face_avatar_");
	std::string id = std::to_string(id_person);
	filename = path + filename + id + std::string(".jpg");
	cv::imwrite(filename, img_avatar);

	//saving descriptor to a file
	/*
	if (debug_mode_flag) 
	{
		std::string id = std::to_string(id_person);
		std::string filename("face_descriptor_");
		filename = filename + id + std::string(".bin");
		//in debug mode we are saving descriptor into a file		
		std::ofstream file(filename.c_str());
		if (!file.is_open()) {  // если файл не открыт
			cout << "Unable to open: " << filename.c_str() << "for writing \n";
			return -1;
		}
		dlib::serialize(res_descriptor, file);
		file.close();
	}*/
	
	//saving descriptor to a MySQL DB
	std::stringbuf buf;
	std::ostream descriptor_stream(&buf);
	//Serialization with Dlib
	dlib::serialize(res_descriptor, descriptor_stream);
	
	//Serialization with Jsoncpp
	Json::Value root;
	root["task_type"] = "descriptor_data";
	root["descriptor_v"] = Json::Value(Json::arrayValue);
	for(int i=0; i<res_descriptor.size(); i++)
	   root["descriptor_v"].append(res_descriptor(0, i));
	
	cout << root <<endl;
	
	Json::FastWriter fastWriter;
	std::string json_string = fastWriter.write(root);
	
	//test for unparsing
/*
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
	
	for(int i=0; i<128; i++)
	{
	  std::string x_str = test_root["descriptor_v"][i].asString();
	  float x=atof(x_str.c_str());
	  std::cout << x<< endl;
	  //std::cout << test_root["descriptor_v"][i] << endl;
	}
*/	
	if (debug_mode_flag) 
	{
		std::string id = std::to_string(id_person);
		std::string filename("face_descriptor_");
		filename = filename + id + std::string(".json");
		//in debug mode we are saving descriptor into a file
		std::ofstream file(filename.c_str());
		//std::ofstream file;
		//file.open(filename.c_str(), ios::out | ios::binary);
		
		//if (!file.open(filename.c_str(), ios::out | ios::binary)) {  // если файл не открыт
		if (!file.is_open()){  // если файл не открыт
		    cout << "Unable to open: " << filename.c_str() << "for writing \n";
		   return -1;
		}
		
		//dlib::serialize(res_descriptor, file);
		//Json::StyledWriter styledWriter;
		//file << styledWriter.write(root);
		Json::FastWriter fastWriter;
		std::string output = fastWriter.write(root);
		file.write(output.c_str(), sizeof(char)*output.size());
		file.close();
	}

	
	//std::string face_hash = buf.str();		
	std::string face_hash = json_string;
	std::string insert_query;  
	//insert_query = "insert into person(id, face_hash, voice_hash, avatar,name) values('" + id + "','" + buf.c_str() + "','cda','aaa','uasya');";
	
	int size = strlen(face_hash.c_str());
	char chunk[2*size+1];
	mysql_real_escape_string(connection, chunk, face_hash.c_str(), size);

	//char *st = "INSERT INTO person(id, face_hash, voice_hash, avatar,name) VALUES('%i','%s','cda','aaa','uasya');";
	char* st="UPDATE person SET face_hash='%s' WHERE id=%i";
	size_t st_len = strlen(st);
	char query[st_len + 2*size+1]; 
	int len = snprintf(query, st_len + 2*size+1, st, chunk ,id_person);
	
	//cout << "Inserting into database len=" << st_len << " face_hash: " << face_hash.c_str() <<endl;
	
	if (mysql_query(connection, query)) 
	{
	  finish_with_error(connection);
	}
	
	mysql_close(connection);
		
	cout << "{status:""OK""}" << endl;
	
	return 0;
}

catch (std::exception& e)
{
	cout << e.what() << endl;
}

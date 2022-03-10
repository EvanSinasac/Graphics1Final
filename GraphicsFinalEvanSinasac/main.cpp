//Evan Sinasac - 1081418
//INFO6028 Graphics (Final)
//main.cpp description:
//				The purpose of the final is to apply everything we've done in the course and make a dungeon	

#include "GLCommon.h"
#include "GLMCommon.h"
//Shader stuff in global things
#include "globalThings.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>


float lastX = 600.0f;
float lastY = 320.0f;
bool firstMouse = true;
float cameraYaw = 90.0f;
float cameraPitch = 0.0f;

double deltaTime = 0.0;

int numberOfTransparentObjects = 0;

std::vector<std::string> modelLocations;


//Function signature for DrawObject()
void DrawObject(
	cMesh* pCurrentMesh,
	glm::mat4 matModel,
	GLint matModel_Location,
	GLint matModelInverseTranspose_Location,
	GLuint program,
	cVAOManager* pVAOManager);

//callbacks
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

bool loadWorldFile();		//not used for final
bool loadLightsFile();
std::string g_grid[51][65];
bool loadTSVGrid();

void loadFinalModels();

int main(int argc, char** argv)
{
	GLFWwindow* pWindow;

	GLuint program = 0;		//0 means no shader program

	GLint mvp_location = -1;
	std::stringstream ss;
	float baseAtten = 0.75f;
	float randomAddedAtten;

	glfwSetErrorCallback(GLFW_error_callback);

	if (!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	pWindow = glfwCreateWindow(1200, 640, "Graphics Project 2!", NULL, NULL);

	if (!pWindow)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(pWindow, GLFW_key_callback);

	glfwSetCursorEnterCallback(pWindow, GLFW_cursor_enter_callback);
	glfwSetCursorPosCallback(pWindow, GLFW_cursor_position_callback);
	glfwSetScrollCallback(pWindow, GLFW_scroll_callback);
	glfwSetMouseButtonCallback(pWindow, GLFW_mouse_button_callback);
	glfwSetWindowSizeCallback(pWindow, GLFW_window_size_callback);


	glfwSetCursorPosCallback(pWindow, mouse_callback);

	glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(pWindow);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);


	GLint max_uniform_location = 0;
	GLint* p_max_uniform_location = NULL;
	p_max_uniform_location = &max_uniform_location;
	glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, p_max_uniform_location);

	std::cout << "GL_MAX_UNIFORM_LOCATIONS: " << max_uniform_location << std::endl;

	// Create global things
	::g_StartUp(pWindow);

	cShaderManager::cShader vertShader;
	ss.str("");
	ss << SOLUTION_DIR << "common\\assets\\shaders\\vertShader_01.glsl";
	vertShader.fileName = ss.str();

	cShaderManager::cShader geomShader;
	ss.str("");
	ss << SOLUTION_DIR << "common\\assets\\shaders\\geomShader.glsl";
	geomShader.fileName = ss.str();;

	cShaderManager::cShader fragShader;
	ss.str("");
	ss << SOLUTION_DIR << "common\\assets\\shaders\\fragShader_01.glsl";
	fragShader.fileName = ss.str();

	if (::g_pShaderManager->createProgramFromFile("Shader#1", vertShader, geomShader, fragShader))
	{
		std::cout << "Shader compiled OK" << std::endl;
		// 
		// Set the "program" variable to the one the Shader Manager used...
		program = ::g_pShaderManager->getIDFromFriendlyName("Shader#1");
	}
	else
	{
		std::cout << "Error making shader program: " << std::endl;
		std::cout << ::g_pShaderManager->getLastError() << std::endl;
	}


	// Select the shader program we want to use
	// (Note we only have one shader program at this point)
	glUseProgram(program);

	// *******************************************************
	// Now, I'm going to "load up" all the uniform locations
	// (This was to show how a map could be used)
	cShaderManager::cShaderProgram* pShaderProc = ::g_pShaderManager->pGetShaderProgramFromFriendlyName("Shader#1");
	int theUniformIDLoc = -1;
	theUniformIDLoc = glGetUniformLocation(program, "matModel");

	pShaderProc->mapUniformName_to_UniformLocation["matModel"] = theUniformIDLoc;

	// Or...
	pShaderProc->mapUniformName_to_UniformLocation["matModel"] = glGetUniformLocation(program, "matModel");

	pShaderProc->mapUniformName_to_UniformLocation["matView"] = glGetUniformLocation(program, "matView");
	pShaderProc->mapUniformName_to_UniformLocation["matProjection"] = glGetUniformLocation(program, "matProjection");
	pShaderProc->mapUniformName_to_UniformLocation["matModelInverseTranspose"] = glGetUniformLocation(program, "matModelInverseTranspose");

	pShaderProc->mapUniformName_to_UniformLocation["bDiscardTransparencyWindowsOn"] = glGetUniformLocation(program, "bDiscardTransparencyWindowsOn");

	pShaderProc->mapUniformName_to_UniformLocation["eyeLocation"] = glGetUniformLocation(program, "eyeLocation");

	// *******************************************************

	// TODO: GO through all these and load them into the map instead, and then go through the errors and pull the location from the map instead
	//GLint mvp_location = -1;
	mvp_location = glGetUniformLocation(program, "MVP");

	// Get "uniform locations" (aka the registers these are in)
	GLint matModel_Location = glGetUniformLocation(program, "matModel");
	//	GLint matView_Location = glGetUniformLocation(program, "matView");
	GLint matProjection_Location = glGetUniformLocation(program, "matProjection");
	GLint matModelInverseTranspose_Location = glGetUniformLocation(program, "matModelInverseTranspose");

	GLint bDiscardTransparencyWindowsON_LocID = glGetUniformLocation(program, "bDiscardTransparencyWindowsOn");

	//Lights stuff here

//    	vec4 param1;	// x = lightType, y = inner angle, z = outer angle, w = TBD
//	                // 0 = pointlight
//					// 1 = spot light
//					// 2 = directional light

	// Large scene light	CAN'T BE INDEX 0!!!!  Index 0 seems to overwrite all other lights, so it can be the spotlight or something, not sure why,
	// might be something in the shader

	// loads all the lights from a file instead of individually setting them here
	if (loadLightsFile())
	{
		std::cout << "loadLightsFile finished ok!" << std::endl;
	}
	else
	{
		std::cout << "loadLightsFile did NOT finish ok!  Aborting!" << std::endl;
		return -1;
	}

	if (loadTSVGrid())
	{
		std::cout << "loadTSVGrid finished ok!" << std::endl;
	}
	else
	{
		std::cout << "loadTSVGrid did NOT finish ok!  Aborting!" << std::endl;
		return -1;
	}

	// Get the uniform locations of the light shader values
	::g_pTheLights->SetUpUniformLocations(program);

	ss.str("");
	//ss << SOLUTION_DIR << "common\\assets\\models\\";
	ss << SOLUTION_DIR << "common\\assets\\models\\";

	::g_pVAOManager->setFilePath(ss.str());


	// Load ALL the models
	//loadMidtermModels();
	//loadProject2Models();
	loadFinalModels();


	unsigned int totalVerticesLoaded = 0;
	unsigned int totalTrianglesLoaded = 0;
	for (std::vector<std::string>::iterator itModel = modelLocations.begin(); itModel != modelLocations.end(); itModel++)
	{
		sModelDrawInfo theModel;
		std::string modelName = *itModel;
		std::cout << "Loading " << modelName << "...";
		if (!::g_pVAOManager->LoadModelIntoVAO(modelName, theModel, program))
		{
			std::cout << "didn't work because: " << std::endl;
			std::cout << ::g_pVAOManager->getLastError(true) << std::endl;
		}
		else
		{
			std::cout << "OK." << std::endl;
			std::cout << "\t" << theModel.numberOfVertices << " vertices and " << theModel.numberOfTriangles << " triangles loaded." << std::endl;
			totalTrianglesLoaded += theModel.numberOfTriangles;
			totalVerticesLoaded += theModel.numberOfVertices;
		}
	} //end of for (std::vector<std::string>::iterator itModel

	std::cout << "Done loading models." << std::endl;
	std::cout << "Total vertices loaded = " << totalVerticesLoaded << std::endl;
	std::cout << "Total triangles loaded = " << totalTrianglesLoaded << std::endl;

	//indexes 0-7 for opening and closing doors

	//World file stuff here
	if (loadWorldFile())
	{
		std::cout << "loadWorldFile finished OK" << std::endl;
	}
	else
	{
		std::cout << "loadWorldFile did not finish OK, aborting" << std::endl;
		return -1;
	}

	//Michael Feeney's BMP texture mapping
	//// Load the textures
	ss.str("");
	ss << SOLUTION_DIR << "common\\assets\\textures\\";
	::g_pTextureManager->SetBasePath(ss.str());

	if (::g_pTextureManager->Create2DTextureFromBMPFile("Dungeons_2_Texture_01_A.bmp", true))
		//if (::g_pTextureManager->Create2DTextureFromBMPFile("Pebbleswithquarzite.bmp", true))
	{
		std::cout << "Loaded the texture" << std::endl;
	}
	else
	{
		std::cout << "DIDN'T load the texture" << std::endl;
	}

	if (::g_pTextureManager->Create2DTextureFromBMPFile("FX_SunBeam_Tex.bmp", true))
	{
		std::cout << "Loaded the texture" << std::endl;
	}
	else
	{
		std::cout << "DIDN't load the texture" << std::endl;
	}

	// Default don't have a texture
	if (::g_pTextureManager->Create2DTextureFromBMPFile("BrightColouredUVMap.bmp", true))
		//if (::g_pTextureManager->Create2DTextureFromBMPFile("Pebbleswithquarzite.bmp", true))
	{
		std::cout << "Loaded the texture" << std::endl;
	}
	else
	{
		std::cout << "DIDN'T load the texture" << std::endl;
	}

	// Add a skybox texture
	std::string errorTextString;
	ss.str("");
	ss << SOLUTION_DIR << "common\\assets\\textures\\cubemaps\\";
	::g_pTextureManager->SetBasePath(ss.str());		// update base path to cube texture location

	if (!::g_pTextureManager->CreateCubeTextureFromBMPFiles("TropicalSunnyDay",
		"TropicalSunnyDayRight2048.bmp",	/* +X */	"TropicalSunnyDayLeft2048.bmp" /* -X */,
		"TropicalSunnyDayUp2048.bmp",		/* +Y */	"TropicalSunnyDayDown2048.bmp" /* -Y */,
		"TropicalSunnyDayFront2048.bmp",	/* +Z */	"TropicalSunnyDayBack2048.bmp" /* -Z */,
		true, errorTextString))
	{
		std::cout << "Didn't load because: " << errorTextString << std::endl;
	}


	if (!::g_pTextureManager->CreateCubeTextureFromBMPFiles("Space01",
		"SpaceBox_right1_posX.bmp",		/* +X */	"SpaceBox_left2_negX.bmp" /* -X */,
		"SpaceBox_top3_posY.bmp",		/* +Y */	"SpaceBox_bottom4_negY.bmp" /* -Y */,
		"SpaceBox_front5_posZ.bmp",		/* +Z */	"SpaceBox_back6_negZ.bmp" /* -Z */,
		true, errorTextString))
	{
		std::cout << "Didn't load because: " << errorTextString << std::endl;
	}

	if (!::g_pTextureManager->CreateCubeTextureFromBMPFiles("City01",	
		"city_lf.bmp",		/* +X */	"city_rt.bmp" /* -X */,
		"city_dn.bmp",		/* +Y */	"city_up.bmp" /* -Y */,
		"city_ft.bmp",		/* +Z */	"city_bk.bmp" /* -Z */,
		true, errorTextString))
	{
		std::cout << "Didn't load because: " << errorTextString << std::endl;
	}

	//// Turns out to make a "cube" map, the images need to be squares
	if (!::g_pTextureManager->CreateCubeTextureFromBMPFiles("Skyrim",
		"Skyrim_Right.bmp",		/* +X */	"Skyrim_Left.bmp" /* -X */,
		"Skyrim_Top.bmp",		/* +Y */	"Skyrim_Bottom.bmp" /* -Y */,
		"Skyrim_Forward.bmp",		/* +Z */	"Skyrim_Back.bmp" /* -Z */,
		true, errorTextString))
	{
		std::cout << "Didn't load because: " << errorTextString << std::endl;
	}

	// Make sphere for "skybox" before world file
	cMesh* sphereSky = new cMesh();
	//sphereSky->meshName = "Isosphere_Smooth_Normals.ply";
	// We are using a sphere with INWARD facing normals, so we see the "back" of the sphere
	sphereSky->meshName = "Isosphere_Smooth_Inverted_Normals_for_SkyBox.ply";
	// 2 main ways we can do a skybox:
	//
	// - Make a sphere really big, so everything fits inside
	// (be careful of the far clipping plane)
	//
	// - Typical way is:
	//	- Turn off the depth test
	//	- Turn off the depth function (i.e. writing to the depth buffer)
	//	- Draw the skybox object (which can be really small, since it's not interacting with the depth buffer)
	//	- Once drawn:
	//		- Turn on the depth function
	//		- Turn on the depth test

	sphereSky->positionXYZ = ::cameraEye;	//glm::vec3(100.0f, -250.0f, 100.0f);
	//sphereSky->orientationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	sphereSky->scale = 10000.0f;
	//sphereSky->bUseWholeObjectDiffuseColour = false;
	//sphereSky->wholeObjectDiffuseRGBA = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	sphereSky->bDontLight = true;
	sphereSky->clearTextureRatiosToZero();
	//sphereSky->textureNames[1] = "2k_jupiter.bmp";
	sphereSky->textureRatios[1] = 1.0f;
	//Draw it separately so don't add it to the vector
	//::g_vec_pMeshes.push_back(sphereSky);

	

	// My camera reset stuff	(might be broken, idk anymore)
	if (::cameraEye.x > 0 && ::cameraEye.z > 0)
	{
		::cameraYaw = 180.f + (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (::cameraEye.x > 0 && ::cameraEye.z < 0)
	{
		::cameraYaw = 90.f - (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (::cameraEye.x < 0 && ::cameraEye.z > 0)
	{
		::cameraYaw = (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (::cameraEye.x < 0 && ::cameraEye.z < 0)
	{
		::cameraYaw = (atan(::cameraEye.z / ::cameraEye.x) * 180.f / glm::pi<float>());
	}
	else if (::cameraEye.x == 0.f)
	{
		if (::cameraEye.z >= 0.f)
		{
			::cameraYaw = 270.f;
		}
		else
		{
			::cameraYaw = 90.f;
		}
	}
	else if (::cameraEye.z == 0.f)
	{
		if (::cameraEye.x <= 0)
		{
			::cameraYaw = 0.f;
		}
		else
		{
			::cameraYaw = 180.f;
		}
	}
	//anyways, after figuring out the yaw, we set the target at the negative of the xz of the ::camera position and y=0 (this faces the ::camera towards the origin)
	::cameraTarget = glm::vec3(-1.f * ::cameraEye.x, 0, -1.f * ::cameraEye.z);
	glm::normalize(::cameraTarget);

	const double MAX_DELTA_TIME = 0.1;	//100 ms
	double previousTime = glfwGetTime();

	while (!glfwWindowShouldClose(pWindow))
	{
		float ratio;
		int width, height;
		glm::mat4 matModel;				// used to be "m"; Sometimes it's called "world"
		glm::mat4 matProjection;        // used to be "p";
		glm::mat4 matView;              // used to be "v";

		double currentTime = glfwGetTime();
		deltaTime = currentTime - previousTime;
		deltaTime = (deltaTime > MAX_DELTA_TIME ? MAX_DELTA_TIME : deltaTime);
		previousTime = currentTime;

		glfwGetFramebufferSize(pWindow, &width, &height);
		ratio = width / (float)height;

		//Turn on the depth buffer
		glEnable(GL_DEPTH);         // Turns on the depth buffer
		glEnable(GL_DEPTH_TEST);    // Check if the pixel is already closer

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// *******************************************************
		// Screen is cleared and we are ready to draw the scene...
		// *******************************************************

		// Update the title text
		glfwSetWindowTitle(pWindow, ::g_TitleText.c_str());

		// Before sending the light info from the shader, we have to get those torches flickering
		for (unsigned int index = 1; index < 52; index++)
		{
			srand(time(NULL));
			randomAddedAtten = (rand() % 100) /100.0f;
			::g_pTheLights->theLights[index].atten.y = baseAtten * gGetRandBetween<float>(1.0f, 2.0f); //wtf?  what was the point of the random ^?
		}

		// Copy the light information into the shader to draw the scene
		::g_pTheLights->CopyLightInfoToShader();

		matProjection = glm::perspective(0.6f,	// FOV variable later
			ratio,
			0.1f,								// Near plane
			1'000'000.0f);						// Far plane

		matView = glm::mat4(1.0f);
		matView = glm::lookAt(::cameraEye,
			::cameraEye + ::cameraTarget,
			upVector);

		glUniformMatrix4fv(pShaderProc->getUniformID_From_Name("matView"),
			1, GL_FALSE, glm::value_ptr(matView));


		glUniformMatrix4fv(matProjection_Location, 1, GL_FALSE, glm::value_ptr(matProjection));

		glUniform4f(pShaderProc->mapUniformName_to_UniformLocation["eyeLocation"], ::cameraEye.x, ::cameraEye.y, ::cameraEye.z, 1.0f);

		// ***************************************
		// Geometry shader generate normals
		//  uniform bool bDrawNormals;
		//  uniform float debugNormalLength;

		GLint bDrawDebugNormals_LocID = glGetUniformLocation(program, "bDrawDebugNormals");
		if (::g_bShowDebugObjects)
		{
			glUniform1f(bDrawDebugNormals_LocID, (GLfloat)GL_TRUE);

			GLint debugNormalLength_LocID = glGetUniformLocation(program, "debugNormalLength");
			glUniform1f(debugNormalLength_LocID, 0.1f);
		}
		else
		{
			glUniform1f(bDrawDebugNormals_LocID, (GLfloat)GL_FALSE);
		}
		// ***************************************



		// Since this is a space game (most of the screen is filled with "sky"),
		// I'll draw the skybox first

		GLint bIsSkyBox_LocID = glGetUniformLocation(program, "bIsSkyBox");
		glUniform1f(bIsSkyBox_LocID, (GLfloat)GL_TRUE);

		// Move the "skybox object" with the camera
		sphereSky->positionXYZ = ::cameraEye;
		DrawObject(sphereSky, glm::mat4(1.0f),
			matModel_Location, matModelInverseTranspose_Location,
			program, ::g_pVAOManager);

		glUniform1f(bIsSkyBox_LocID, (GLfloat)GL_FALSE);

		//For transparency for the beakers, they need to be drawn farthest from the camera first, so we need to sort the last 4 meshes
		// Alpha transparency sorting
		for (unsigned int index = ::g_vec_pMeshes.size() - numberOfTransparentObjects; index != g_vec_pMeshes.size() - 1; index++)
		{
			//Distance is the difference in length between camera eye and mesh position
			float dist1, dist2;
			dist1 = glm::abs(glm::length(cameraEye - ::g_vec_pMeshes[index]->positionXYZ));
			dist2 = glm::abs(glm::length(cameraEye - ::g_vec_pMeshes[index + 1]->positionXYZ));
			//if the second object is farther, move it up
			if (dist2 > dist1)
			{
				cMesh* temp = ::g_vec_pMeshes[index + 1];
				::g_vec_pMeshes[index + 1] = ::g_vec_pMeshes[index];
				::g_vec_pMeshes[index] = temp;
			}

		}

		// TODO: Open/close the doors
		//if (::openDoors)
		//{

		//}
		//if (::closeDoors)
		//{

		//}
		//if (::openPortculli)
		//{

		//}
		//if (::closePortculli)
		//{

		//}

		// **********************************************************************
		// Draw the "scene" of all objects.
		// i.e. go through the vector and draw each one...
		// **********************************************************************
		for (unsigned int index = 0; index != ::g_vec_pMeshes.size(); index++)
		{
			// So the code is a little easier...
			cMesh* pCurrentMesh = ::g_vec_pMeshes[index];

			matModel = glm::mat4(1.0f);  // "Identity" ("do nothing", like x1)
			//mat4x4_identity(m);

			if (pCurrentMesh->bHasDiscardTexture)
			{
				// Discard texture
				{
					//GLuint discardTextureNumber = ::g_pTextureManager->getTextureIDFromName("Lisse_mobile_shipyard-mal1.bmp");
					GLuint discardTextureNumber = ::g_pTextureManager->getTextureIDFromName(pCurrentMesh->discardTexture);

					// I'm picking texture unit 30 since it's not in use.
					GLuint textureUnit = 30;
					glActiveTexture(textureUnit + GL_TEXTURE0);
					glBindTexture(GL_TEXTURE_2D, discardTextureNumber);
					GLint discardTexture_LocID = glGetUniformLocation(program, "discardTexture");
					glUniform1i(discardTexture_LocID, textureUnit);


					// Turn it on
					glUniform1f(bDiscardTransparencyWindowsON_LocID, (GLfloat)GL_TRUE);
					DrawObject(pCurrentMesh,
						matModel,
						matModel_Location,
						matModelInverseTranspose_Location,
						program,
						::g_pVAOManager);

					// Turn it off
					glUniform1f(pShaderProc->mapUniformName_to_UniformLocation["bDiscardTransparencyWindowsOn"], (GLfloat)GL_FALSE);
				}
			}
			/*else if (glm::distance(::cameraEye, pCurrentMesh->positionXYZ) < 100.0f)
			{
				DrawObject(pCurrentMesh,
					matModel,
					matModel_Location,
					matModelInverseTranspose_Location,
					program,
					::g_pVAOManager);
			}*/
			else if (glm::distance(::cameraEye, pCurrentMesh->positionXYZ) > 50.0f)
			{
				if (!pCurrentMesh->bIsWireframe)
				{
					pCurrentMesh->bIsWireframe = true;
					for (std::vector<cMesh*>::iterator it = pCurrentMesh->vec_pChildMeshes.begin();
						it != pCurrentMesh->vec_pChildMeshes.end(); it++)
					{
						(*it)->bIsWireframe = true;
						for (std::vector<cMesh*>::iterator iT = (*it)->vec_pChildMeshes.begin();
							iT != (*it)->vec_pChildMeshes.end(); iT++)
						{
							(*iT)->bIsWireframe = true;
						}
					}
				}
				DrawObject(pCurrentMesh,
					matModel,
					matModel_Location,
					matModelInverseTranspose_Location,
					program,
					::g_pVAOManager);
			}
			else
			{
				if (pCurrentMesh->bIsWireframe != ::nearbyWireFrame)
				{
					pCurrentMesh->bIsWireframe = ::nearbyWireFrame;
					for (std::vector<cMesh*>::iterator it = pCurrentMesh->vec_pChildMeshes.begin();
						it != pCurrentMesh->vec_pChildMeshes.end(); it++)
					{
						(*it)->bIsWireframe = ::nearbyWireFrame;
						for (std::vector<cMesh*>::iterator iT = (*it)->vec_pChildMeshes.begin();
							iT != (*it)->vec_pChildMeshes.end(); iT++)
						{
							(*iT)->bIsWireframe = ::nearbyWireFrame;
						}
					}
				}
				DrawObject(pCurrentMesh,
					matModel,
					matModel_Location,
					matModelInverseTranspose_Location,
					program,
					::g_pVAOManager);
			}
				
			
		}//for (unsigned int index


		// Scene is drawn
		// **********************************************************************

		// "Present" what we've drawn.
		glfwSwapBuffers(pWindow);        // Show what we've drawn

		// Process any events that have happened
		glfwPollEvents();

		// Handle OUR keyboard, mouse stuff
		handleAsyncKeyboard(pWindow, deltaTime);
		handleAsyncMouse(pWindow, deltaTime);

	} //end of while (!glfwWindowShouldClose(window))

	// All done, so delete things...
	::g_ShutDown(pWindow);


	glfwDestroyWindow(pWindow);

	glfwTerminate();
	exit(EXIT_SUCCESS);

} //end of main


// Modified from previous versions to take texture information
bool loadWorldFile()
{
	std::stringstream ss;
	std::stringstream sFile;


	ss << SOLUTION_DIR << "common\\assets\\worldFile.txt";

	std::ifstream theFile(ss.str());

	if (!theFile.is_open())
	{
		fprintf(stderr, "Could not open worldFile.txt");
		return false;
	}

	std::string nextToken;
	ss.str("");

	//Throwaway text describing the format of the file
	theFile >> nextToken;       //ModelFileName(extension)
	theFile >> nextToken;       //Position(x,y,z)
	theFile >> nextToken;       //Orientation(x,y,z)
	theFile >> nextToken;       //Scale
	theFile >> nextToken;       //Colour(r,g,b,a)
	theFile >> nextToken;		//Light
	theFile >> nextToken;		//TextureOp
	theFile >> nextToken;		//TextureName1
	theFile >> nextToken;		//Ratio1
	theFile >> nextToken;		//TextureName2
	theFile >> nextToken;		//Ratio2
	theFile >> nextToken;		//TextureName3
	theFile >> nextToken;		//Ratio3
	theFile >> nextToken;		//DiscardTexture

	theFile >> nextToken;		//Camera(x,y,z)
	theFile >> nextToken;		//x position
	::cameraEye.x = std::stof(nextToken);
	theFile >> nextToken;		//y position
	::cameraEye.y = std::stof(nextToken);
	theFile >> nextToken;		//z position
	::cameraEye.z = std::stof(nextToken);

	theFile >> nextToken;		//number of transparent objects
	numberOfTransparentObjects = std::stoi(nextToken);

	//From here modify based on worldFile format
	while (theFile >> nextToken)    //this should always be the name of the model to load or end.  Potential error check, add a check for "ply" in the mdoel name
	{
		cMesh* curMesh = new cMesh;
		if (nextToken == "end")
		{
			break;
		}
		std::cout << nextToken << std::endl;        //Printing model names to console, just making sure we're loading ok.  Can be commented out whenever
		//First is the file name of model
		//ss << SOLUTION_DIR << "common\\assets\\models\\" << nextToken;		// don't need this with the setBasePath function for the loading intot the VAO
		curMesh->meshName = nextToken;
		//Next 3 are the position of the model
		theFile >> nextToken;                                               //x position for the model
		curMesh->positionXYZ.x = std::stof(nextToken);
		theFile >> nextToken;                                               //y position for the model
		curMesh->positionXYZ.y = std::stof(nextToken);
		theFile >> nextToken;                                               //z position for the model
		curMesh->positionXYZ.z = std::stof(nextToken);
		//Next 3 are the orientation of the model
		theFile >> nextToken;                                               //x orientation value
		//curMesh.orientationXYZ.x = std::stof(nextToken);
		curMesh->orientationXYZ.x = glm::radians(std::stof(nextToken));
		theFile >> nextToken;                                               //y orientation value
		//curMesh.orientationXYZ.y = std::stof(nextToken);
		curMesh->orientationXYZ.y = glm::radians(std::stof(nextToken));
		theFile >> nextToken;                                               //z orientation value
		//curMesh.orientationXYZ.z = std::stof(nextToken);
		curMesh->orientationXYZ.z = glm::radians(std::stof(nextToken));
		//Next is the scale to multiply the model by
		theFile >> nextToken;                                               //scale multiplier
		curMesh->scale = std::stof(nextToken);
		//Next 3 are the r, g, b values for the model
		curMesh->bUseWholeObjectDiffuseColour = false;
		theFile >> nextToken;													//RGB red value
		curMesh->wholeObjectDiffuseRGBA.r = std::stof(nextToken) / 255.0f;		//convert to nice shader value (between 0 and 1)
		theFile >> nextToken;													//RGB green value
		curMesh->wholeObjectDiffuseRGBA.g = std::stof(nextToken) / 255.0f;		//convert to nice shader value (between 0 and 1)
		theFile >> nextToken;													//RGB blue value
		curMesh->wholeObjectDiffuseRGBA.b = std::stof(nextToken) / 255.0f;		//convert to nice shader value (between 0 and 1)
		theFile >> nextToken;
		curMesh->wholeObjectDiffuseRGBA.a = std::stof(nextToken);
		curMesh->alphaTransparency = std::stof(nextToken);

		// Light
		theFile >> nextToken;
		curMesh->bDontLight = (nextToken == "0");		// set to true or "0" in the worldFile to see textures without lighting

		// Texture operator
		theFile >> nextToken;
		curMesh->textureOperator = std::stoi(nextToken);

		// texture for mesh is in worldFile.txt now
		curMesh->clearTextureRatiosToZero();

		theFile >> nextToken;
		if (nextToken != "null")
		{
			curMesh->textureNames[1] = nextToken;
			theFile >> nextToken;
			curMesh->textureRatios[1] = std::stof(nextToken);
		}
		else
		{
			theFile >> nextToken;	// throw away the ratio number
		}

		theFile >> nextToken;
		if (nextToken != "null")
		{
			curMesh->textureNames[2] = nextToken;
			theFile >> nextToken;
			curMesh->textureRatios[2] = std::stof(nextToken);
		}
		else
		{
			theFile >> nextToken;	// throw away the ratio number
		}

		theFile >> nextToken;
		if (nextToken != "null")
		{
			curMesh->textureNames[3] = nextToken;
			theFile >> nextToken;
			curMesh->textureRatios[3] = std::stof(nextToken);
		}
		else
		{
			theFile >> nextToken;	// throw away the ratio number
		}

		if (curMesh->textureRatios[1] == 0.0f && curMesh->textureRatios[2] == 0.0f && curMesh->textureRatios[3] == 0.0f)
		{
			curMesh->textureRatios[0] = 1.0f;
		}

		// Discard texture
		theFile >> nextToken;
		if (nextToken != "null")
		{
			curMesh->discardTexture = nextToken;
			curMesh->bHasDiscardTexture = true;
		}

		::g_vec_pMeshes.push_back(curMesh);     //push the model onto our vector of meshes
		ss.str("");                         //reset the stringstream
	} //end of while
	theFile.close();
	return true;
}	//end of load world file

bool loadLightsFile()
{
	std::stringstream ss;
	std::stringstream sFile;


	ss << SOLUTION_DIR << "common\\assets\\lights.txt";

	std::ifstream theFile(ss.str());

	if (!theFile.is_open())
	{
		fprintf(stderr, "Could not open lights.txr");
		return false;
	}

	std::string nextToken;
	ss.str("");

	// Throw away description 
	theFile >> nextToken;		// position(x,y,z)
	theFile >> nextToken;		// diffuse(r,g,b)
	theFile >> nextToken;		// specular(r,g,b)
	theFile >> nextToken;		// atten(x,y,z,w)
	theFile >> nextToken;		// direction(x,y,z)
	theFile >> nextToken;		// param1(x,y,z)
	theFile >> nextToken;		// param2(x)

	::g_pTheLights->TurnOffLight(0);

	unsigned int index = 1;	// can't start at 0 because for some reason the 0 light over writes all other lights

	while (theFile >> nextToken)
	{
		if (nextToken == "end" || index >= cLightManager::NUMBER_OF_LIGHTS)
		{
			break;
		}
		glm::vec3 position;
		glm::vec3 diffuse;
		glm::vec3 specular;
		glm::vec4 atten;
		glm::vec3 direction;
		glm::vec3 param1;
		float param2;

		// Position
		position.x = std::stof(nextToken);
		theFile >> nextToken;
		position.y = std::stof(nextToken);
		theFile >> nextToken;
		position.z = std::stof(nextToken);

		// Diffuse
		theFile >> nextToken;
		diffuse.x = std::stof(nextToken) / 255.0f;
		theFile >> nextToken;
		diffuse.y = std::stof(nextToken) / 255.0f;
		theFile >> nextToken;
		diffuse.z = std::stof(nextToken) / 255.0f;

		// Specular
		theFile >> nextToken;
		specular.x = std::stof(nextToken) / 255.0f;
		theFile >> nextToken;
		specular.y = std::stof(nextToken) / 255.0f;
		theFile >> nextToken;
		specular.z = std::stof(nextToken) / 255.0f;

		// Atten
		theFile >> nextToken;
		atten.x = std::stof(nextToken);
		theFile >> nextToken;
		atten.y = std::stof(nextToken);
		theFile >> nextToken;
		atten.z = std::stof(nextToken);
		theFile >> nextToken;
		atten.w = std::stof(nextToken);

		// Direction
		theFile >> nextToken;
		direction.x = std::stof(nextToken);
		theFile >> nextToken;
		direction.y = std::stof(nextToken);
		theFile >> nextToken;
		direction.z = std::stof(nextToken);
		direction = glm::normalize(direction);

		// Param1
		theFile >> nextToken;
		param1.x = std::stof(nextToken);;
		theFile >> nextToken;
		param1.y = std::stof(nextToken);
		theFile >> nextToken;
		param1.z = std::stof(nextToken);

		// Param2
		theFile >> nextToken;
		param2 = std::stof(nextToken);


		// Load everything into the lights
		::g_pTheLights->theLights[index].position = glm::vec4(position, 1.0f);
		::g_pTheLights->theLights[index].diffuse = glm::vec4(diffuse, 1.0f);
		::g_pTheLights->theLights[index].specular = glm::vec4(specular, 1.0f);
		::g_pTheLights->theLights[index].atten = glm::vec4(atten);
		::g_pTheLights->theLights[index].direction = glm::vec4(direction, 1.0f);
		::g_pTheLights->theLights[index].param1 = glm::vec4(param1, 1.0f);
		::g_pTheLights->theLights[index].param2 = glm::vec4(param2, 0.0f, 0.0f, 1.0f);

		index++;

	} //end of while

	theFile.close();
	return true;

} //end of load lights

bool loadTSVGrid()
{
	float scale = 0.01f;
	std::stringstream ss;
	ss << DUNGEON_DIR << "The Catacombs of Horrendous Devastation 01 - Converted.tsv";
	std::ifstream theFile(ss.str());

	if (!theFile.is_open())
	{
		std::cout << "The file didn't open!" << std::endl;
	}
	std::string nextLetter;
	for (unsigned int y = 0; y < 65; y++)
	{
		for (unsigned int x = 0; x < 51; x++)
		{
			nextLetter = theFile.get();
			g_grid[x][y] = nextLetter;
			if (nextLetter == "D")
			{
				nextLetter = theFile.get();
				if (nextLetter == "P" || nextLetter == "S")
				{
					g_grid[x][y].append(nextLetter);
				}
				else
				{
					x++;
					g_grid[x][y] = nextLetter;
				}
			}
			else if (nextLetter == "S")
			{
				nextLetter = theFile.get();
				if (nextLetter == "D" || nextLetter == "U")
				{
					g_grid[x][y].append(nextLetter);
					nextLetter = theFile.get();
					if (nextLetter == "D" || nextLetter == "U")
					{
						g_grid[x][y].append(nextLetter);
					}
					else
					{
						x++;
						g_grid[x][y] = nextLetter;
					}
				}
				else
				{
					x++;
					g_grid[x][y] = nextLetter;
				}
			}
		} //end of for x
		// Newline
		theFile.get();
	} //end of for y

	

	// D - door
	// F - floor
	// DP - portcullis door
	// DS - secret door
	// SD/SDD - down stairs
	// SU/SUU - up stairs
	// - - wall
	
	// Choose the mesh and location depending on the x/y values of the grid
	for (unsigned int y = 0; y < 65; y++)
	{
		for (unsigned int x = 0; x < 51; x++)
		{
			//std::cout << g_grid[x][y] << " ";
			cMesh* newMesh;

			// First check what kind of model we need, depends on the letter stored in the mesh
			if (g_grid[x][y] == "-")
			{
				// skip, we'll do the walls depending on the floor
			} 
			else if (g_grid[x][y] == "D")
			{
				// We need to check what the orientation of the door is
				newMesh = new cMesh();

				newMesh->meshName = "SM_Env_Dwarf_Floor_08.ply"; // "SM_Env_Dwarf_Wall_DoorFrame_Single_01.ply";

				cMesh* doorFrame = new cMesh();
				doorFrame->meshName = "SM_Env_Dwarf_Wall_DoorFrame_Single_01.ply";
				doorFrame->positionXYZ = glm::vec3(-500.0f, 0.0f, 0.0f);
				//orientation
				doorFrame->clearTextureRatiosToZero();
				doorFrame->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				doorFrame->textureRatios[1] = 1.0f;
				doorFrame->textureOperator = 1;

				cMesh* doorMesh = new cMesh();
				doorMesh->meshName = "SM_Env_Dwarf_Wall_DoorFrame_Single_01_Door.ply";
				doorMesh->positionXYZ = glm::vec3(160.0f, 0.0f, 0.0f);
				// orientation
				doorMesh->clearTextureRatiosToZero();
				doorMesh->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				doorMesh->textureRatios[1] = 1.0f;
				doorMesh->textureOperator = 1;
				doorFrame->vec_pChildMeshes.push_back(doorMesh);

				newMesh->vec_pChildMeshes.push_back(doorFrame);

				
				// Check where the walls of the floor are
				if (g_grid[x - 1][y] == "-")
				{
					cMesh* eastWall = new cMesh();
					eastWall->meshName = "SM_Env_Dwarf_Wall_04.ply";
					eastWall->positionXYZ = glm::vec3(-500.0f, 0.0f, 0.0f);
					eastWall->orientationXYZ = glm::vec3(0.0f, glm::radians(270.0f), 0.0f);
					eastWall->clearTextureRatiosToZero();
					eastWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					eastWall->textureRatios[1] = 1.0f;
					eastWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(eastWall);
				}
				if (g_grid[x + 1][y] == "-")
				{
					cMesh* westWall = new cMesh();
					westWall->meshName = "SM_Env_Dwarf_Wall_01.ply";
					westWall->positionXYZ = glm::vec3(0.0f, 0.0f, -500.0f);
					// orientation
					westWall->orientationXYZ = glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
					westWall->clearTextureRatiosToZero();
					westWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					westWall->textureRatios[1] = 1.0f;
					westWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(westWall);
				}
				if (g_grid[x][y - 1] == "-")
				{
					cMesh* southWall = new cMesh();
					southWall->meshName = "SM_Env_Dwarf_Wall_02.ply";
					southWall->positionXYZ = glm::vec3(-500.0f, 0.0f, -500.0f);
					southWall->orientationXYZ = glm::vec3(0.0f, glm::radians(180.0f), 0.0f);
					southWall->clearTextureRatiosToZero();
					southWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					southWall->textureRatios[1] = 1.0f;
					southWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(southWall);
				}
				if (g_grid[x][y + 1] == "-")
				{
					cMesh* northWall = new cMesh();
					northWall->meshName = "SM_Env_Dwarf_Wall_03.ply";
					//doorMesh->positionXYZ = glm::vec3(160.0f, 0.0f, 0.0f);
					// orientation
					northWall->clearTextureRatiosToZero();
					northWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					northWall->textureRatios[1] = 1.0f;
					northWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(northWall);
				}
					
				newMesh->scale = scale;
				// Position
				newMesh->positionXYZ.x = 0.0f + (5 * x);
				newMesh->positionXYZ.y = 0.0f;
				newMesh->positionXYZ.z = 0.0f + (5 * y);
				// Check up/down and left/right, doors should only be in one orientation or the other
				if (g_grid[x - 1][y] == "F")	// if the left is floor, the right should also be floor
				{
					// and we need to rotate the door frame
					//newMesh->orientationXYZ = glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
					newMesh->vec_pChildMeshes[0]->orientationXYZ += glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
					//newMesh->vec_pChildMeshes[0]->positionXYZ += glm::vec3(0.0f, 0.0f, 250.0f);
					// Might have to shift the position, but we'll see
				} //otherwise up and down is floor and we don't need to rotate

				newMesh->bDontLight = false;
				newMesh->clearTextureRatiosToZero();
				newMesh->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				newMesh->textureRatios[1] = 1.0f;
				newMesh->textureOperator = 1;
				::g_vec_pMeshes.push_back(newMesh);
			}
			else if (g_grid[x][y] == "DS")
			{
				// For some reason, the dungeon that was generated for me, even though it had arches in it,
				// the tsv file doesn't have them marked, but the secret doors are, so I'm putting arches where the secret doors are
				newMesh = new cMesh();

				newMesh->meshName = "SM_Env_Dwarf_Floor_12.ply"; // "SM_Env_Dwarf_Wall_DoorFrame_Single_01.ply";

				cMesh* doorFrame = new cMesh();
				doorFrame->meshName = "SM_Env_Dwarf_Wall_Archway_02.ply";
				doorFrame->positionXYZ = glm::vec3(-500.0f, 0.0f, 0.0f);
				//orientation
				doorFrame->clearTextureRatiosToZero();
				doorFrame->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				doorFrame->textureRatios[1] = 1.0f;
				doorFrame->textureOperator = 1;
				
				// Check where the walls of the floor are
				if (g_grid[x - 1][y] == "-")
				{
					cMesh* eastWall = new cMesh();
					eastWall->meshName = "SM_Env_Dwarf_Wall_04.ply";
					eastWall->positionXYZ = glm::vec3(-500.0f, 0.0f, 0.0f);
					eastWall->orientationXYZ = glm::vec3(0.0f, glm::radians(270.0f), 0.0f);
					eastWall->clearTextureRatiosToZero();
					eastWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					eastWall->textureRatios[1] = 1.0f;
					eastWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(eastWall);
				}
				if (g_grid[x + 1][y] == "-")
				{
					cMesh* westWall = new cMesh();
					westWall->meshName = "SM_Env_Dwarf_Wall_01.ply";
					westWall->positionXYZ = glm::vec3(0.0f, 0.0f, -500.0f);
					// orientation
					westWall->orientationXYZ = glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
					westWall->clearTextureRatiosToZero();
					westWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					westWall->textureRatios[1] = 1.0f;
					westWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(westWall);
				}
				if (g_grid[x][y - 1] == "-")
				{
					cMesh* southWall = new cMesh();
					southWall->meshName = "SM_Env_Dwarf_Wall_02.ply";
					southWall->positionXYZ = glm::vec3(-500.0f, 0.0f, -500.0f);
					southWall->orientationXYZ = glm::vec3(0.0f, glm::radians(180.0f), 0.0f);
					southWall->clearTextureRatiosToZero();
					southWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					southWall->textureRatios[1] = 1.0f;
					southWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(southWall);
				}
				if (g_grid[x][y + 1] == "-")
				{
					cMesh* northWall = new cMesh();
					northWall->meshName = "SM_Env_Dwarf_Wall_03.ply";
					//doorMesh->positionXYZ = glm::vec3(160.0f, 0.0f, 0.0f);
					// orientation
					northWall->clearTextureRatiosToZero();
					northWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					northWall->textureRatios[1] = 1.0f;
					northWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(northWall);
				}

				newMesh->scale = scale;
				// Position
				newMesh->positionXYZ.x = 0.0f + (5 * x);
				newMesh->positionXYZ.y = 0.0f;
				newMesh->positionXYZ.z = 0.0f + (5 * y);
				// Check up/down and left/right, doors should only be in one orientation or the other
				if (g_grid[x - 1][y] == "F")	// if the left is floor, the right should also be floor
				{
					// and we need to rotate the door frame
					//newMesh->orientationXYZ = glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
					newMesh->vec_pChildMeshes[0]->orientationXYZ += glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
					//newMesh->vec_pChildMeshes[0]->positionXYZ += glm::vec3(0.0f, 0.0f, 250.0f);
					// Might have to shift the position, but we'll see
				} //otherwise up and down is floor and we don't need to rotate

				newMesh->bDontLight = false;
				newMesh->clearTextureRatiosToZero();
				newMesh->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				newMesh->textureRatios[1] = 1.0f;
				newMesh->textureOperator = 1;
				::g_vec_pMeshes.push_back(newMesh);
			}
			else if (g_grid[x][y] == "DP")
			{
			// Portcullis
				newMesh = new cMesh();

				newMesh->meshName = "SM_Env_Dwarf_Floor_05.ply"; // "SM_Env_Dwarf_Wall_DoorFrame_Single_01.ply";

				cMesh* doorFrame = new cMesh();
				doorFrame->meshName = "SM_Env_Dwarf_Wall_DoorFrame_Slider_01.ply";
				doorFrame->positionXYZ = glm::vec3(-500.0f, 0.0f, 0.0f);
				//orientation
				doorFrame->clearTextureRatiosToZero();
				doorFrame->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				doorFrame->textureRatios[1] = 1.0f;
				doorFrame->textureOperator = 1;

				cMesh* doorMesh = new cMesh();
				doorMesh->meshName = "SM_Env_Dwarf_Wall_DoorFrame_Slider_01_Door.ply";
				doorMesh->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
				doorMesh->friendlyName = "portcullis";
				// orientation
				doorMesh->clearTextureRatiosToZero();
				doorMesh->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				doorMesh->textureRatios[1] = 1.0f;
				doorMesh->textureOperator = 1;
				doorFrame->vec_pChildMeshes.push_back(doorMesh);

				newMesh->vec_pChildMeshes.push_back(doorFrame);


				// Check where the walls of the floor are
				if (g_grid[x - 1][y] == "-")
				{
					cMesh* eastWall = new cMesh();
					eastWall->meshName = "SM_Env_Dwarf_Wall_04.ply";
					eastWall->positionXYZ = glm::vec3(-500.0f, 0.0f, 0.0f);
					eastWall->orientationXYZ = glm::vec3(0.0f, glm::radians(270.0f), 0.0f);
					eastWall->clearTextureRatiosToZero();
					eastWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					eastWall->textureRatios[1] = 1.0f;
					eastWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(eastWall);
				}
				if (g_grid[x + 1][y] == "-")
				{
					cMesh* westWall = new cMesh();
					westWall->meshName = "SM_Env_Dwarf_Wall_01.ply";
					westWall->positionXYZ = glm::vec3(0.0f, 0.0f, -500.0f);
					// orientation
					westWall->orientationXYZ = glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
					westWall->clearTextureRatiosToZero();
					westWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					westWall->textureRatios[1] = 1.0f;
					westWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(westWall);
				}
				if (g_grid[x][y - 1] == "-")
				{
					cMesh* southWall = new cMesh();
					southWall->meshName = "SM_Env_Dwarf_Wall_02.ply";
					southWall->positionXYZ = glm::vec3(-500.0f, 0.0f, -500.0f);
					southWall->orientationXYZ = glm::vec3(0.0f, glm::radians(180.0f), 0.0f);
					southWall->clearTextureRatiosToZero();
					southWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					southWall->textureRatios[1] = 1.0f;
					southWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(southWall);
				}
				if (g_grid[x][y + 1] == "-")
				{
					cMesh* northWall = new cMesh();
					northWall->meshName = "SM_Env_Dwarf_Wall_03.ply";
					//doorMesh->positionXYZ = glm::vec3(160.0f, 0.0f, 0.0f);
					// orientation
					northWall->clearTextureRatiosToZero();
					northWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					northWall->textureRatios[1] = 1.0f;
					northWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(northWall);
				}

				newMesh->scale = scale;
				// Position
				newMesh->positionXYZ.x = 0.0f + (5 * x);
				newMesh->positionXYZ.y = 0.0f;
				newMesh->positionXYZ.z = 0.0f + (5 * y);
				// Check up/down and left/right, doors should only be in one orientation or the other
				if (g_grid[x - 1][y] == "F")	// if the left is floor, the right should also be floor
				{
					// and we need to rotate the door frame
					//newMesh->orientationXYZ = glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
					newMesh->vec_pChildMeshes[0]->orientationXYZ += glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
					//newMesh->vec_pChildMeshes[0]->positionXYZ += glm::vec3(0.0f, 0.0f, 250.0f);
					// Might have to shift the position, but we'll see
				} //otherwise up and down is floor and we don't need to rotate

				newMesh->bDontLight = false;
				newMesh->clearTextureRatiosToZero();
				newMesh->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				newMesh->textureRatios[1] = 1.0f;
				newMesh->textureOperator = 1;
				::g_vec_pMeshes.push_back(newMesh);
			}
			else if (g_grid[x][y] == "SD")
			{
				// Check the orientation, use half stairs (maybe)
				newMesh = new cMesh();
				newMesh->meshName = "SM_Env_Dwarf_Stairs_Half_01.ply";
				newMesh->scale = scale;
				// Position
				newMesh->positionXYZ.x = 0.0f + (5 * x) - 5.0f;
				newMesh->positionXYZ.y = -1.25f;
				newMesh->positionXYZ.z = 0.0f + (5 * y) - 2.5f;
				newMesh->orientationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
				newMesh->bDontLight = false;
				newMesh->clearTextureRatiosToZero();
				newMesh->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				newMesh->textureRatios[1] = 1.0f;
				newMesh->textureOperator = 1;
				::g_vec_pMeshes.push_back(newMesh);
			}
			else if (g_grid[x][y] == "SDD")
			{
				// Check the orientation, use full stairs
				newMesh = new cMesh();
				newMesh->meshName = "SM_Env_Dwarf_Stairs_01.ply";
				newMesh->scale = scale;
				// Position
				newMesh->positionXYZ.x = 0.0f + (5 * x) - 5.0f;
				newMesh->positionXYZ.y = -3.75f;
				newMesh->positionXYZ.z = 0.0f + (5 * y) - 2.5f;
				newMesh->orientationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
				newMesh->bDontLight = false;
				newMesh->clearTextureRatiosToZero();
				newMesh->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				newMesh->textureRatios[1] = 1.0f;
				newMesh->textureOperator = 1;
				::g_vec_pMeshes.push_back(newMesh);
			}
			else if (g_grid[x][y] == "SU")
			{
				// Check the orientation, use half stairs
				newMesh = new cMesh();
				newMesh->meshName = "SM_Prop_Camp_Wood_Stairs_01.ply";
				newMesh->scale = scale;
				// Position
				newMesh->positionXYZ.x = 0.0f + (5 * x) - 2.5f;
				newMesh->positionXYZ.y = 0.0f;
				newMesh->positionXYZ.z = 0.0f + (5 * y);
				newMesh->orientationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
				newMesh->bDontLight = false;
				newMesh->clearTextureRatiosToZero();
				newMesh->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				newMesh->textureRatios[1] = 1.0f;
				newMesh->textureOperator = 1;
				::g_vec_pMeshes.push_back(newMesh);
			}
			else if (g_grid[x][y] == "SUU")
			{
				// Check the orientation, use full stairs
				newMesh = new cMesh();
				newMesh->meshName = "SM_Prop_Camp_Wood_Stairs_02.ply";
				newMesh->scale = scale;
				// Position
				newMesh->positionXYZ.x = 0.0f + (5 * x) - 2.5f;
				newMesh->positionXYZ.y = 0.75f;
				newMesh->positionXYZ.z = 0.0f + (5 * y) + 3.75f;
				newMesh->orientationXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
				newMesh->bDontLight = false;
				newMesh->clearTextureRatiosToZero();
				newMesh->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				newMesh->textureRatios[1] = 1.0f;
				newMesh->textureOperator = 1;
				::g_vec_pMeshes.push_back(newMesh);
			}
			else if (g_grid[x][y] == "F")
			{
				// Check the foor positions from here and place walls alongside the floor
				newMesh = new cMesh();
				int numOfWalls = 0;
				// Check where the walls of the floor are
				if (g_grid[x - 1][y] == "-")
				{
					cMesh* eastWall = new cMesh();
					eastWall->meshName = "SM_Env_Dwarf_Wall_04.ply";
					eastWall->positionXYZ = glm::vec3(-500.0f, 0.0f, 0.0f);
					eastWall->orientationXYZ = glm::vec3(0.0f, glm::radians(270.0f), 0.0f);
					eastWall->clearTextureRatiosToZero();
					eastWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					eastWall->textureRatios[1] = 1.0f;
					eastWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(eastWall);
					numOfWalls++;
				}
				if (g_grid[x + 1][y] == "-")
				{
					cMesh* westWall = new cMesh();
					westWall->meshName = "SM_Env_Dwarf_Wall_01.ply";
					westWall->positionXYZ = glm::vec3(0.0f, 0.0f, -500.0f);
					// orientation
					westWall->orientationXYZ = glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
					westWall->clearTextureRatiosToZero();
					westWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					westWall->textureRatios[1] = 1.0f;
					westWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(westWall);
					numOfWalls++;
				}
				if (g_grid[x][y - 1] == "-")
				{
					cMesh* southWall = new cMesh();
					southWall->meshName = "SM_Env_Dwarf_Wall_02.ply";
					southWall->positionXYZ = glm::vec3(-500.0f, 0.0f, -500.0f);
					southWall->orientationXYZ = glm::vec3(0.0f, glm::radians(180.0f), 0.0f);
					southWall->clearTextureRatiosToZero();
					southWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					southWall->textureRatios[1] = 1.0f;
					southWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(southWall);
					numOfWalls++;
				}
				if (g_grid[x][y + 1] == "-")
				{
					cMesh* northWall = new cMesh();
					northWall->meshName = "SM_Env_Dwarf_Wall_03.ply";
					//doorMesh->positionXYZ = glm::vec3(160.0f, 0.0f, 0.0f);
					// orientation
					northWall->clearTextureRatiosToZero();
					northWall->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
					northWall->textureRatios[1] = 1.0f;
					northWall->textureOperator = 1;
					newMesh->vec_pChildMeshes.push_back(northWall);
					numOfWalls++;
				}

				switch (numOfWalls)
				{
				case 0:
					newMesh->meshName = "SM_Env_Dwarf_Floor_01.ply";
					break;
				case 1:
					newMesh->meshName = "SM_Env_Dwarf_Floor_02.ply";
					break;
				case 2:
					newMesh->meshName = "SM_Env_Dwarf_Floor_03.ply";
					break;
				case 3:
					newMesh->meshName = "SM_Env_Dwarf_Floor_04.ply";
					break;
				case 4:
					newMesh->meshName = "SM_Env_Dwarf_Floor_05.ply";
					break;
				default:
					newMesh->meshName = "SM_Env_Dwarf_Floor_06.ply";
				}

				newMesh->scale = scale;
				// Position
				newMesh->positionXYZ.x = 0.0f + (5 * x);
				newMesh->positionXYZ.y = 0.0f;
				newMesh->positionXYZ.z = 0.0f + (5 * y);
				newMesh->bDontLight = false;
				newMesh->clearTextureRatiosToZero();
				newMesh->textureNames[1] = "Dungeons_2_Texture_01_A.bmp";
				newMesh->textureRatios[1] = 1.0f;
				newMesh->textureOperator = 1;
				::g_vec_pMeshes.push_back(newMesh);
				
			}
		}
	}

	theFile.close();
	return true;
}

//Figured out the math for how to do this from https://learnopengl.com/Getting-started/Camera and http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
//Using the mouse position we calculate the direction that the camera will be facing
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	//if it's the start of the program this smooths out a potentially glitchy jump
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	//find the offset of where the mouse positions have moved
	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	//multiply by sensitivity so that it's not potentially crazy fast
	float sensitivity = 0.1f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	::cameraYaw += xOffset;         // The yaw is the rotation around the ::camera's y-axis (so we want to add the xOffset to it)
	::cameraPitch += yOffset;       // The pitch is the rotation around the ::camera's x-axis (so we want to add the yOffset to it)
	//This limits the pitch so that we can't just spin the ::camera under/over itself
	if (::cameraPitch > 89.0f)
		::cameraPitch = 89.0f;
	if (::cameraPitch < -89.0f)
		::cameraPitch = -89.0f;
	//calculations for the new direction based on the mouse movements
	glm::vec3 direction;
	direction.x = cos(glm::radians(::cameraYaw)) * cos(glm::radians(::cameraPitch));
	direction.y = sin(glm::radians(::cameraPitch));
	direction.z = sin(glm::radians(::cameraYaw)) * cos(glm::radians(::cameraPitch));
	::cameraTarget = glm::normalize(direction);
} //fly ::camera


void loadFinalModels()
{
	modelLocations.push_back("Isosphere_Smooth_Inverted_Normals_for_SkyBox.ply");
	// Models for the final project
	modelLocations.push_back("FX_SunShafts.ply");

	modelLocations.push_back("SM_Bld_Dwarf_Building_Door_01.ply");
	modelLocations.push_back("SM_Bld_Dwarf_Building_Door_02.ply");
	modelLocations.push_back("SM_Bld_Dwarf_Building_Door_03.ply");

	modelLocations.push_back("SM_Env_Bramble_01.ply");
	modelLocations.push_back("SM_Env_Bramble_02.ply");
	modelLocations.push_back("SM_Env_Bramble_03.ply");
	modelLocations.push_back("SM_Env_Bush_01.ply");

	modelLocations.push_back("SM_Env_Crystals_Cluster_Large_01.ply");
	modelLocations.push_back("SM_Env_Crystals_Cluster_Large_02.ply");
	modelLocations.push_back("SM_Env_Crystals_Cluster_Large_03.ply");
	modelLocations.push_back("SM_Env_Crystals_Cluster_Large_04.ply");

	modelLocations.push_back("SM_Env_Dwarf_Floor_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_02.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_03.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_04.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_05.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_06.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_07.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_08.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_09.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_10.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_11.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_12.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_13.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_14.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_15.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_16.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_17.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_18.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_19.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_20.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_21.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_22.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_23.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_24.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_25.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_26.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_27.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_28.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_29.ply");

	modelLocations.push_back("SM_Env_Dwarf_Floor_Corner_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_Half_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Floor_Half_02.ply");
	modelLocations.push_back("SM_Env_Dwarf_Gate_01_Door_L.ply");
	modelLocations.push_back("SM_Env_Dwarf_Gate_01_Door_R.ply");

	modelLocations.push_back("SM_Env_Dwarf_Stairs_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Stairs_Half_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Stairs_Small_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Stairs_Small_Corner_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Stairs_Small_Corner_01_Alt.ply");
	modelLocations.push_back("SM_Env_Dwarf_Stairs_Small_Corner_02.ply");
	modelLocations.push_back("SM_Env_Dwarf_Stairs_Small_Corner_02_Alt.ply");
	modelLocations.push_back("SM_Env_Dwarf_Stairs_Small_Corner_03.ply");
	modelLocations.push_back("SM_Env_Dwarf_Stairs_Small_Corner_04.ply");
	modelLocations.push_back("SM_Env_Dwarf_Stairs_Small_Corner_05.ply");

	modelLocations.push_back("SM_Env_Dwarf_Wall_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_02.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_03.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_04.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_05.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_06.ply");

	modelLocations.push_back("SM_Env_Dwarf_Wall_Archway_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Archway_02.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Archway_03.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Archway_04.ply");

	modelLocations.push_back("SM_Env_Dwarf_Wall_Broken_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Broken_02.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Broken_03.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Broken_04.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Broken_05.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Broken_06.ply");

	modelLocations.push_back("SM_Env_Dwarf_Wall_DoorFrame_Double_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_DoorFrame_Double_01_Door_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_DoorFrame_Double_02.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_DoorFrame_Double_02_Door_02.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_DoorFrame_Single_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_DoorFrame_Single_01_Door.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_DoorFrame_Single_02.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_DoorFrame_Single_02_Door.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_DoorFrame_Slider_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_DoorFrame_Slider_01_Door.ply");

	modelLocations.push_back("SM_Env_Dwarf_Wall_Tunnel_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Tunnel_Archway_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Window_01.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Window_02.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Window_03.ply");
	modelLocations.push_back("SM_Env_Dwarf_Wall_Window_04.ply");

	modelLocations.push_back("SM_Env_Fern_01.ply");

	modelLocations.push_back("SM_Env_Floor_Grate_01.ply");
	modelLocations.push_back("SM_Env_Flowers_01.ply");
	modelLocations.push_back("SM_Env_Flowers_02.ply");

	modelLocations.push_back("SM_Env_Grass_01.ply");
	modelLocations.push_back("SM_Env_Grass_02.ply");
	modelLocations.push_back("SM_Env_Grass_03.ply");

	modelLocations.push_back("SM_Env_Grass_Thin_01.ply");
	modelLocations.push_back("SM_Env_Grass_Thin_02.ply");
	modelLocations.push_back("SM_Env_Grass_Thin_03.ply");
	modelLocations.push_back("SM_Env_Grass_Thin_04.ply");

	modelLocations.push_back("SM_Env_Grass_Tuft_01.ply");
	modelLocations.push_back("SM_Env_Grass_Tuft_02.ply");
	modelLocations.push_back("SM_Env_Grass_Tuft_03.ply");
	modelLocations.push_back("SM_Env_Grass_Tuft_04.ply");

	modelLocations.push_back("SM_Env_GrassPatch_01.ply");
	modelLocations.push_back("SM_Env_GrassPatch_02.ply");
	modelLocations.push_back("SM_Env_GrassPatch_03.ply");

	modelLocations.push_back("SM_Env_GroundLeaves_01.ply");
	modelLocations.push_back("SM_Env_GroundLeaves_02.ply");
	modelLocations.push_back("SM_Env_GroundLeaves_03.ply");

	modelLocations.push_back("SM_Env_Mangrove_Tree_01.ply");
	modelLocations.push_back("SM_Env_Mangrove_Tree_02.ply");

	modelLocations.push_back("SM_Env_PalmBush_03.ply");
	modelLocations.push_back("SM_Env_PalmTree_01.ply");
	modelLocations.push_back("SM_Env_PalmTree_02.ply");
	modelLocations.push_back("SM_Env_PalmTree_03.ply");
	modelLocations.push_back("SM_Env_PalmTree_04.ply");
	modelLocations.push_back("SM_Env_PalmTree_Tall_01.ply");
	modelLocations.push_back("SM_Env_PalmTree_Tall_02.ply");
	modelLocations.push_back("SM_Env_Plant_01.ply");
	modelLocations.push_back("SM_Env_Plant_02.ply");

	modelLocations.push_back("SM_Env_Plant_Spikey_01.ply");
	modelLocations.push_back("SM_Env_Plants_01.ply");
	modelLocations.push_back("SM_Env_Plants_02.ply");
	modelLocations.push_back("SM_Env_Plants_03.ply");

	modelLocations.push_back("SM_Env_Rock_Stairs_01.ply");
	modelLocations.push_back("SM_Env_Rock_Stairs_02.ply");
	modelLocations.push_back("SM_Env_Rock_Stairs_03.ply");
	modelLocations.push_back("SM_Env_Rock_Stairs_04.ply");

	modelLocations.push_back("SM_Env_Tree_Base_01.ply");
	modelLocations.push_back("SM_Env_Tree_Base_02.ply");
	modelLocations.push_back("SM_Env_Tree_Big_01.obj.ply");

	modelLocations.push_back("SM_Env_Tree_Canopy_01.ply");
	modelLocations.push_back("SM_Env_Tree_Canopy_02.ply");
	modelLocations.push_back("SM_Env_Tree_Canopy_03.ply");
	modelLocations.push_back("SM_Env_Tree_Canopy_04.ply");

	modelLocations.push_back("SM_Env_Tree_Dead_01.ply");
	modelLocations.push_back("SM_Env_Tree_Large_01.ply");
	modelLocations.push_back("SM_Env_Tree_Large_02.ply");

	modelLocations.push_back("SM_Env_Tree_Vines_01.ply");
	modelLocations.push_back("SM_Env_Tree_Vines_02.ply");
	modelLocations.push_back("SM_Env_Tree_Vines_03.ply");
	modelLocations.push_back("SM_Env_Tree_Vines_04.ply");
	modelLocations.push_back("SM_Env_Tree_Vines_05.ply");

	modelLocations.push_back("SM_Env_Vines_Wall_01.ply");
	modelLocations.push_back("SM_Env_Vines_Wall_02.ply");

	modelLocations.push_back("SM_Generic_Tree_01.ply");
	modelLocations.push_back("SM_Generic_Tree_02.ply");
	modelLocations.push_back("SM_Generic_Tree_03.ply");
	modelLocations.push_back("SM_Generic_Tree_04.ply");
	modelLocations.push_back("SM_Generic_TreeDead_01.ply");
	modelLocations.push_back("SM_Generic_TreeStump_01.ply");

	modelLocations.push_back("SM_Prop_Camp_Wood_Stairs_01.ply");
	modelLocations.push_back("SM_Prop_Camp_Wood_Stairs_02.ply");

	modelLocations.push_back("SM_Prop_Crystal_Rock_A_01.ply");
	modelLocations.push_back("SM_Prop_Crystal_Rock_A_03.ply");
	modelLocations.push_back("SM_Prop_Crystal_Rock_B_01.ply");
	modelLocations.push_back("SM_Prop_Crystal_Rock_B_02.ply");
	modelLocations.push_back("SM_Prop_Crystal_Rock_B_03.ply");

	modelLocations.push_back("SM_Prop_Dwarf_Torch_01.ply");
	modelLocations.push_back("SM_Prop_Dwarf_Torch_02.ply");
	modelLocations.push_back("SM_Prop_Dwarf_Torch_03.ply");
	modelLocations.push_back("SM_Prop_Dwarf_Torch_04.ply");
	modelLocations.push_back("SM_Prop_Dwarf_Torch_05.ply");
	modelLocations.push_back("SM_Prop_Dwarf_Torch_06.ply");

	modelLocations.push_back("SM_Prop_Gem_01.ply");
	modelLocations.push_back("SM_Prop_Gem_03.ply");
	modelLocations.push_back("SM_Prop_Gem_03.ply");
	modelLocations.push_back("SM_Prop_Gem_04.ply");
	modelLocations.push_back("SM_Prop_Gem_05.ply");
	modelLocations.push_back("SM_Prop_Gem_06.ply");
	modelLocations.push_back("SM_Prop_Gem_07.ply");

	modelLocations.push_back("SM_Prop_Gem_Cluster_01.ply");
	modelLocations.push_back("SM_Prop_Gem_Cluster_02.ply");
	modelLocations.push_back("SM_Prop_Gem_Cluster_03.ply");

	modelLocations.push_back("SM_Prop_Gem_Socket_01.ply");
	modelLocations.push_back("SM_Prop_Gem_Socket_02.ply");
	modelLocations.push_back("SM_Prop_Gem_Socket_03.ply");
	modelLocations.push_back("SM_Prop_Gem_Socket_04.ply");
	modelLocations.push_back("SM_Prop_Gem_Socket_05.ply");

	modelLocations.push_back("SM_Prop_Rug_Stairs_01.ply");
	modelLocations.push_back("SM_Prop_Rug_Stairs_02.ply");
	modelLocations.push_back("SM_Prop_Rug_Stairs_03.ply");
}


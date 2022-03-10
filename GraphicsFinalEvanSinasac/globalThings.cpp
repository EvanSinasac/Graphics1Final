#include "globalThings.h"
//Stuff done in class with Michael Feeney

//cFlyCamera* g_pFlyCamera = NULL;

bool openDoors = false;
bool closeDoors = false;
bool openPortculli = false;
bool closePortculli = false;

bool nearbyWireFrame = false;

//glm::vec3 g_cameraEye = glm::vec3(0.0f, 0.0f, +4.0f);
//glm::vec3 g_cameraAt = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraEye = glm::vec3(0.0f, 2.0f, -4.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

cVAOManager* g_pVAOManager = NULL;
cShaderManager* g_pShaderManager = NULL;
cBasicTextureManager* g_pTextureManager = NULL;

cLightManager* g_pTheLights = NULL;
// This got moved into the light manager class
//extern cLightHelper* g_pTheLightHelper;


// List of objects to draw
std::vector< cMesh* > g_vec_pMeshes;

//std::map<unsigned int, std::vector<glm::vec3>> g_debugNormals;
bool g_turnDebugNormalsOn = false;


cMesh* g_pDebugSphere = NULL;
bool g_bShowDebugShere = true;

bool g_bShowDebugObjects = false;


unsigned int g_selectedObject = 0;
unsigned int g_selectedLight = 3;       //3rd light in the vector (blue spotlight over bed)

// Used for the texture height displacement example
glm::vec3 g_heightMapUVOffsetRotation = glm::vec3(0.0f, 0.0f, 0.0f);

// This will be printed in the title bar
std::string g_TitleText = "";





bool g_FindObjectByUniqueID(unsigned int uniqueID_toFind, cMesh*& theObjectIFound);



cMesh* g_findObjectByUniqueID(unsigned int uniqueID_toFind)
{
    for (unsigned int index = 0; index != ::g_vec_pMeshes.size(); index++)
    {
        if (::g_vec_pMeshes[index]->getUniqueID() == uniqueID_toFind)
        {
            return ::g_vec_pMeshes[index];
        }
    }
    return NULL;
}

bool g_findObjectByUniqueID(unsigned int uniqueID_toFind, cMesh& theObjectIFound)
{
    for (unsigned int index = 0; index != ::g_vec_pMeshes.size(); index++)
    {
        if (::g_vec_pMeshes[index]->getUniqueID() == uniqueID_toFind)
        {
            // Dereferences and returns a stack based copy
            theObjectIFound = *(::g_vec_pMeshes[index]);
            return true;
        }
    }
    return false;
}

bool g_FindObjectByUniqueID(unsigned int uniqueID_toFind, cMesh*& theObjectIFound)
{
    for (unsigned int index = 0; index != ::g_vec_pMeshes.size(); index++)
    {
        if (::g_vec_pMeshes[index]->getUniqueID() == uniqueID_toFind)
        {
            // Returns a pointer to the object
            theObjectIFound = ::g_vec_pMeshes[index];
            return true;
        }
    }
    return false;
}

// If not found, return NULL (0)
cMesh* g_findObjectByFriendlyName(std::string NametoFind)
{
    for (unsigned int index = 0; index != ::g_vec_pMeshes.size(); index++)
    {
        cMesh* pMeshToTest = ::g_vec_pMeshes[index];

        // Is functionally the same thing as using a reference:
        //cMesh& meshToTest = g_vecMeshes[index];

        if (pMeshToTest->friendlyName == NametoFind)
        {
            return pMeshToTest;
        }
    }
    // DIDN'T find it, return NULL
    return NULL;
}

cMesh* g_FindObjectByUniqueID(unsigned int uniqueID_toFind)
{
    for (unsigned int index = 0; index != ::g_vec_pMeshes.size(); index++)
    {
        cMesh* pMeshToTest = ::g_vec_pMeshes[index];

        // Is functionally the same thing as using a reference:
        //cMesh& meshToTest = g_vecMeshes[index];

        if (pMeshToTest->getUniqueID() == uniqueID_toFind)
        {
            return pMeshToTest;
        }
    }
    // DIDN'T find it, return NULL
    return NULL;
}



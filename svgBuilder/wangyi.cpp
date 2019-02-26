//
//  wangyi.cpp
//  Lightning
//
//  Created by Yingying Wang on 7/24/18.
//

#include <stdio.h>
#include "wangyiMoho.h"

void wangyiGetJson(std::string& strCharacterJson, std::string& strComicJson,
                   std::string& strPanelTxt) {
  try {
    Lightning::initialize();
  } catch (std::exception& e) {
    exit(1);
  }

  std::string strComicID = std::string("10227217");
  std::string strCharacterID =
      std::string("219051060");  // std::string("204562765");//
  std::string strComicUrl =
      COMIC_DATA_URL_PREFIX + strComicID + COMIC_DATA_URL_POSTFIX;
  std::string strCharacterUrl = CHARACTER_SERVICE_URL_PREFIX + strCharacterID +
                                CHARACTER_SERVICE_URL_BITMOJI_POSTFIX;

  HTTPDownloader downloader;
  strCharacterJson = downloader.downloadJSON(strCharacterUrl);
  strComicJson = downloader.downloadJSON(strComicUrl);
  std::cout << "*************comic data url: " << strComicUrl << std::endl
            << "*************character data url: " << strCharacterUrl
            << std::endl;

  rapidjson::Document docComic = Lightning::loadJSONFromString(strComicJson);
  rapidjson::StringBuffer strPanel;
  rapidjson::Writer<rapidjson::StringBuffer> panelWriter(strPanel);
  docComic["strips"][0]["panels"][0].Accept(panelWriter);
  strPanelTxt = strPanel.GetString();
};

void wangyiGenAnimation(std::string strInputPath, std::string strOutputPath) {
  BimojiSequence seq;
  seq.LoadAnimationFromMohoExport(strInputPath);
  seq.GenSVGSequence(strOutputPath);
};

void CameraSettings::GetPointOfInterestInPixel(float& x, float& y, float& z) {
  float fCenterX = m_fDocWidth / 2, fCenterY = m_fDocHeight / 2,
        fCenterZ = m_fDocHeight / 2;
  if (m_fAspect > 0) {
    x = ((m_fPosX + 1) * fCenterY) + (fCenterX - fCenterY);
    y = ((m_fPosY * (-1)) + 1) * fCenterY;
    z = ((m_fPosZ * (-1)) * fCenterZ) + 0.1;
  } else {
    x = ((m_fPosX + 1) * fCenterX);
    y = ((m_fPosY * (-1)) + 1) * fCenterX + (fCenterY - fCenterX);
    z = ((m_fPosZ * (-1)) * fCenterX) + 0.1;
  }
};

// HI: EACH LAYER HAS ORGIN, TRANS, ROT AND SCALE AS WELL
// the final bone pos should be
//      POS = Lorigin + (pos - Lorigin) * Lscale + Ltranslate
//
// then use camera to project POS
void CameraSettings::GetProjectionPos(float& x, float& y, float& z) {
  /*Suppose camera by default at (FOV 60deg):
          cam.x, cam.y, cam.z

   Content bimoji position at:
          POS.x, POS.y, POS.z (layer depth)

   Image Size:
          fDocWidth, fDocHeight

   Image Center:
          fImageCenterX = fDocWidth/2, fImageCenterY = fDocHeight/2

   Due to zoom = Base_FoV/FoV; when zoom=1, Base_FOV=60, thus FOV is
          FoV = (60 deg/zoom)

   Camera-Bimoji distance:
          abs(cam.z - POS.z)

   Camera-Image distance:
          focalLength = 0.5*fDocHeight/tan(Fov/2)

   So projection factor is:
          fProjFactor = FL / abs(cam.z-POS.z)

   Project (POS.x, POS.y, POS.z) onto (docWidth, docHeight) image

      imgX = (POS.x-cam.x) * fProjFactor + fDocWidth/2
      imgY = -(POS.y-cam.y) * fProjFactor + fDocHeight/2
  */

  float fBimojiX = x, fBimojiY = y;
  float fProjFactor = GetProjectionFactor(z);
  float fImageCenterX = m_fDocWidth / 2, fImageCenterY = m_fDocHeight / 2;

  x = (fBimojiX - m_fPosX) * fProjFactor + fImageCenterX;
  y = -(fBimojiY - m_fPosY) * fProjFactor + fImageCenterY;
  z = 0;
};

float CameraSettings::GetProjectionFactor(float fLayerZ) {
  float fFOV = (M_PI / 3) / m_fZoom;
  float fTan = tan(fFOV / 2);
  float FL = (m_fDocHeight / 2) / fTan;
  float fProjFactor = FL / abs(m_fPosZ - fLayerZ);
  return fProjFactor;
}

std::string BimojiBoneDOF::s_strDOFName[22] = {
    "B1",    "spine_1", "spine_2", "neck",  "head",  "B21", "bicepR", "foreR",
    "handR", "B20",     "bicepL",  "foreL", "handL", "B19", "thighR", "shinR",
    "footR", "toeR",    "thighL",  "shinL", "footL", "toeL"};

void BimojiSwitchLayerDOF::UpdateEmotionState() {
  m_dEmotion = -1;
  m_dState = -1;

  if (m_strValue.empty()) return;

  int dPos = m_strValue.find(" ");
  std::string strEmotion = m_strValue;
  std::string strState = "";
  if (dPos != std::string::npos) {
    strEmotion = m_strValue.substr(0, dPos);
    strState = m_strValue.substr(dPos + 1);
  }

  if (strEmotion == "neutral")
    m_dEmotion = 1;
  else if (strEmotion == "happy")
    m_dEmotion = 2;
  else if (strEmotion == "frown")
    m_dEmotion = 3;
  else if (strEmotion == "sad")
    m_dEmotion = 3;
  else if (strEmotion == "mad")
    m_dEmotion = 4;
  else if (strEmotion == "shock") {
    if (m_strName == "mouth")
      m_dEmotion = 4;
    else
      m_dEmotion = 5;
  }

  if (!strState.empty()) m_dState = atof(strState.c_str());
}

std::string BimojiFrame::GenPanelString(std::string strPanelBase) {
  std::string strResult;
  // body rotation
  int dPos = strPanelBase.find("\"master_rotation\":");
  dPos = dPos + strlen("\"master_rotation\":");
  strPanelBase.at(dPos) = '0' + m_dBodyOrientation;

  // head rotation
  dPos = strPanelBase.find("\"h_rot\":0,");
  dPos = dPos + strlen("\"h_rot\":");
  strPanelBase.at(dPos) = '0' + m_dHeadOrientation;

  // bone rotation
  int dBegIdx = strPanelBase.find("\"angles\":{");
  int dEndIdx = strPanelBase.find("},", dBegIdx);
  std::string strPrefix = strPanelBase.substr(0, dBegIdx);
  std::string strSuffix = strPanelBase.substr(dEndIdx + 2);
  std::string strAngles = getAnglesString();

  std::cout << "prefix[" << strPrefix << "]" << std::endl;
  std::cout << "suffix[" << strSuffix << "]" << std::endl;
  std::cout << "angles[" << strAngles << "]" << std::endl;
  strResult = strPrefix + strAngles + strSuffix;

  // switch layer
  for (int s = 0; s < 7; ++s) {
    BimojiSwitchLayerDOF sldof = m_arSwitchLayerDOF[s];
    int dPosHead = strResult.find("\"head\":{") + 1;
    if (sldof.m_strName == "mouth") {
      dPos = strResult.find("\"mouth\":", dPosHead) + strlen("\"mouth\":");
      strResult.at(dPos) = '0' + sldof.m_dEmotion;

      dPos = strResult.find("\"lipsync\":", dPosHead) + strlen("\"lipsync\":");
      strResult.at(dPos) = '0' + sldof.m_dState;
    } else if (sldof.m_strName == "eye R") {
      dPos = strResult.find("\"eye_R\":", dPosHead) + strlen("\"eye_R\":");
      strResult.at(dPos) = '0' + sldof.m_dEmotion;
    } else if (sldof.m_strName == "eye L") {
      dPos = strResult.find("\"eye_L\":", dPosHead) + strlen("\"eye_L\":");
      strResult.at(dPos) = '0' + sldof.m_dEmotion;
    } else if (sldof.m_strName == "brow R") {
      dPos = strResult.find("\"brow_R\":", dPosHead) + strlen("\"brow_R\":");
      strResult.at(dPos) = '0' + sldof.m_dEmotion;
    } else if (sldof.m_strName == "brow L") {
      dPos = strResult.find("\"brow_L\":", dPosHead) + strlen("\"brow_L\":");
      strResult.at(dPos) = '0' + sldof.m_dEmotion;
    } else if (sldof.m_strName == "eyelid R") {
      dPos = strResult.find("\"lids\":", dPosHead) + strlen("\"lids\":");
      strResult.at(dPos + 3) = '0' + sldof.m_dState;
    } else if (sldof.m_strName == "eyelid L") {
      dPos = strResult.find("\"lids\":", dPosHead) + strlen("\"lids\":");
      strResult.at(dPos + 1) = '0' + sldof.m_dState;
    }
  }

  // camera settings
  // zoom
  dPos = strResult.find("\"content_scale\":") + strlen("\"content_scale\":");
  strPrefix = strResult.substr(0, dPos);
  dPos = strResult.find(",", dPos);
  strSuffix = strResult.substr(dPos);
  // fZoom here should be fProjFactor w.r.t character proper scale vs moho
  float fZoom = m_cam.GetProjectionFactor(m_layerTrans.m_fPosZ) / 270;
  strResult = strPrefix + std::to_string(fZoom) + strSuffix;

  // camera roll, content_rotation
  dBegIdx =
      strResult.find("\"content_rotation\":") + strlen("\"content_rotation\":");
  strPrefix = strResult.substr(0, dBegIdx);
  dEndIdx = strResult.find(",", dBegIdx);
  if (dEndIdx == std::string::npos) dEndIdx = strResult.find("}", dBegIdx);
  strSuffix = strResult.substr(dEndIdx);
  strResult = strPrefix +
              std::to_string(-m_cam.m_fRoll - m_layerTrans.m_fRotZ) + strSuffix;

  // Doc width
  dBegIdx = strResult.rfind("\"width\":") + strlen("\"width\":");
  strPrefix = strResult.substr(0, dBegIdx);
  dEndIdx = strResult.find(",", dBegIdx);
  strSuffix = strResult.substr(dEndIdx);
  strResult = strPrefix + std::to_string(int(m_cam.m_fDocWidth)) + strSuffix;

  // Doc height
  dBegIdx = strResult.find("\"height\":") + strlen("\"height\":");
  strPrefix = strResult.substr(0, dBegIdx);
  dEndIdx = strResult.find(",", dBegIdx);
  strSuffix = strResult.substr(dEndIdx);
  strResult = strPrefix + std::to_string(int(m_cam.m_fDocHeight)) + strSuffix;

  // content_x,content_y
  float x = 0, y = 0, z = 0;
  GetRootPos(x, y);
  m_cam.GetProjectionPos(x, y, z);
  dBegIdx = strResult.find("\"content_x\":") + strlen("\"content_x\":");
  strPrefix = strResult.substr(0, dBegIdx);
  dEndIdx = strResult.find(",", dBegIdx);
  strSuffix = strResult.substr(dEndIdx);
  strResult = strPrefix + std::to_string(x) + strSuffix;

  dBegIdx = strResult.find("\"content_y\":") + strlen("\"content_y\":");
  strPrefix = strResult.substr(0, dBegIdx);
  dEndIdx = strResult.find(",", dBegIdx);
  strSuffix = strResult.substr(dEndIdx);
  strResult = strPrefix + std::to_string(y) + strSuffix;

  // inside panel x, y
  dBegIdx =
      strResult.find("\"position\":{\"x\":") + strlen("\"position\":{\"x\":");
  strPrefix = strResult.substr(0, dBegIdx);
  dEndIdx = strResult.find(",", dBegIdx);
  strSuffix = strResult.substr(dEndIdx);
  strResult = strPrefix + std::to_string(0) + strSuffix;

  dBegIdx = strResult.find("\"position\":{\"x\":");
  dBegIdx = strResult.find("\"y\":", dBegIdx) + strlen("\"y\":");
  strPrefix = strResult.substr(0, dBegIdx);
  dEndIdx = strResult.find("}", dBegIdx);
  strSuffix = strResult.substr(dEndIdx);
  strResult = strPrefix + std::to_string(0) + strSuffix;

  std::cout << "\n\nPREFIX:[" << strPrefix << "\n\nSUFFIX:[" << strSuffix
            << "\n\nWHOLE:[" << strResult;

  return strResult;
};

void BimojiFrame::GetRootPos(float& x, float& y) {
  // somewhere between two feet
  x = 0, y = -0.3;

  // layer transform: POS = Lorigin + ROTATE(pos - Lorigin) * Lscale +
  // Ltranslate
  float theta = m_layerTrans.m_fRotZ * M_PI / 180;
  float deltaX = x - m_layerTrans.m_fOriginX;
  float deltaY = y - m_layerTrans.m_fOriginY;

  float deltaX2 =
      m_layerTrans.m_fScaleX * (cos(theta) * deltaX - sin(theta) * deltaY);
  float deltaY2 =
      m_layerTrans.m_fScaleY * (sin(theta) * deltaX + cos(theta) * deltaY);

  x = deltaX2 + m_layerTrans.m_fOriginX + m_layerTrans.m_fPosX;
  y = deltaY2 + m_layerTrans.m_fOriginY + m_layerTrans.m_fPosY;
};

void BimojiFrame::GetRootAngle(float& a) { a = m_arBoneDOF[0].m_fRotA - 90; };

std::string BimojiFrame::getAnglesString() {
  std::string strAngles = "";
  for (int d = 0; d < 22; ++d) {
    if (d == BONE_B1 || d == BONE_B21 || d == BONE_B20 || d == BONE_B19)
      continue;

    BimojiBoneDOF dof = m_arBoneDOF[d];
    float fRotA = -m_arBoneDOF[d].m_fRotA;
    if (d == BONE_SP1) {
      float fB1 = (90 - m_arBoneDOF[0].m_fRotA);
      fRotA = fB1 + fRotA;
    }
    if (d == BONE_BICEP_R || d == BONE_BICEP_L || d == BONE_THIGH_R ||
        d == BONE_THIGH_L)
      fRotA = 180 + fRotA;
    if (m_dBodyOrientation &&
        (d == BONE_THIGH_L || d == BONE_SHIN_L || d == BONE_FOOT_L))
      fRotA = 360 - fRotA;

    std::string strDOF = "\"" + dof.m_strName + "\":" + std::to_string(fRotA);
    if (strAngles == "")
      strAngles += strDOF;
    else
      strAngles += "," + strDOF;
  }
  strAngles = "\"angles\":{" + strAngles + "},";
  return strAngles;
}

void BimojiSequence::LoadAnimationFromMohoExport(std::string strInputPath) {
  LoadAnimationFromMohoExportByFrame(strInputPath);
}

void BimojiSequence::LoadAnimationFromMohoExportByFrame(
    std::string strInputPath) {
  std::ifstream fin(strInputPath);
  std::string strLine = "";

  float fDocHeight = 400, fDocWidth = 400, fAspect = 1;
  while (strLine.find("BY FRAME:") == std::string::npos) {
    std::getline(fin, strLine);
    if (strLine.find("Dimension:") == 0) {
      std::string strSub = strLine.substr(strlen("Dimension:"));
      std::stringstream ss(strSub);
      char c;
      ss >> fDocWidth >> c >> fDocHeight >> c >> fAspect;
    }
  }
  do {
    std::string strFrameLine;
    int dColonPos = std::string::npos;
    std::getline(fin, strFrameLine);
    if (strFrameLine.find("BY CHANNEL:") != std::string::npos || fin.eof())
      break;

    BimojiFrame frame;
    int dDOFNum = 0;
    do {
      std::getline(fin, strFrameLine);
      dColonPos = strFrameLine.find(":");
      if (dColonPos == std::string::npos) break;

      std::string strName = strFrameLine.substr(0, dColonPos);
      std::string strValue = strFrameLine.substr(dColonPos + 1);
      if (strName == "camera") {
        CameraSettings& cam = frame.m_cam;
        cam.m_fDocHeight = fDocHeight;
        cam.m_fDocWidth = fDocWidth;
        cam.m_fAspect = fAspect;
        std::stringstream ss(strValue.c_str());
        char cComma;
        ss >> cam.m_fPosX >> cComma >> cam.m_fPosY >> cComma >> cam.m_fPosZ >>
            cComma >> cam.m_fZoom >> cComma >> cam.m_fRoll >> cComma >>
            cam.m_fTilt >> cComma >> cam.m_fPan;
        continue;
      }
      if (strName == "blayer") {
        LayerTransform& lt = frame.m_layerTrans;
        std::stringstream ss(strValue.c_str());
        char cComma;
        ss >> lt.m_fPosX >> cComma >> lt.m_fPosY >> cComma >> lt.m_fPosZ >>
            cComma >> lt.m_fScaleX >> cComma >> lt.m_fScaleY >> cComma >>
            lt.m_fScaleZ >> cComma >> lt.m_fRotX >> cComma >> lt.m_fRotY >>
            cComma >> lt.m_fRotZ >> cComma >> lt.m_fOriginX >> cComma >>
            lt.m_fOriginY;
        continue;
      }
      if (dDOFNum < 22) {
        BimojiBoneDOF boneDof;
        boneDof.m_dIndex = dDOFNum;
        boneDof.m_strName = BimojiBoneDOF::s_strDOFName[dDOFNum];
        int dCommaPos = strValue.find(",");
        boneDof.m_fPosX = atof(strValue.substr(0, dCommaPos).c_str());
        int dCommaPos2 = strValue.find(",", dCommaPos + 1);
        boneDof.m_fPosY = atof(
            strValue.substr(dCommaPos + 1, dCommaPos2 - dCommaPos).c_str());
        boneDof.m_fRotA = atof(strValue.substr(dCommaPos2 + 1).c_str());
        frame.m_arBoneDOF[dDOFNum] = boneDof;
      } else {
        BimojiSwitchLayerDOF slDof;
        slDof.m_dIndex = dDOFNum - 22;
        slDof.m_strName = strName;
        slDof.m_strValue = strValue;
        slDof.UpdateEmotionState();
        frame.m_arSwitchLayerDOF[slDof.m_dIndex] = slDof;
      }
      dDOFNum++;
    } while (true);
    frame.m_pSeq = this;
    m_arFrame.push_back(frame);
  } while (true);
}

void BimojiSequence::GenSVGSequence(
    std::string strOutputPath) {  // generate svg sequences
  std::string strCharacterJson, strComicJson, strPanelTxt;
  wangyiGetJson(strCharacterJson, strComicJson, strPanelTxt);
  Lightning::RequestData requestData;
  requestData.characterData["1"] = strCharacterJson;  //该character
  requestData.isExperimental = true;
  requestData.comicId = std::string("10227217");
  requestData.scale = 1;

  /*Lightning::GeneratedSVGs generatedSVGs;
  Lightning::Metrics perfMetrics;
  for (int f = 0; f < m_arFrame.size(); ++f) {
    Lightning::PerFrameData perFrameData;
    perFrameData.assetLayerStates = std::vector<Lightning::AssetLayerState>(
        3, Lightning::AssetLayerState::ACTIVE);
    perFrameData.comicImagePath = "";
    perFrameData.jsonPanelData = m_arFrame[f].GenPanelString(strPanelTxt);
    // wangyiReplaceAngleInPanelTxt(strPanelTxt,
    // m_arFrame[f].getAnglesString());
    requestData.comicFrameData.push_back(perFrameData);  //每一帧可能不同

    generatedSVGs = Lightning::generateSVGs(requestData, &perfMetrics);
    std::string strSvgPath = strOutputPath + std::to_string(f) + ".svg";
    Lightning::saveStringAsFile(strSvgPath, generatedSVGs.svgs[f]);
  }*/

  bool enableBackground = true;
  std::string backgroundRgbaPath = "";
  if (enableBackground) {
    std::string scalePostfix = "";
    if (requestData.scale != 1) {
      scalePostfix = "_" + std::to_string(requestData.scale);
    }

    std::string fgBgImageUrl = Lightning::FG_BG_IMAGE_URL_PREFIX +
                               requestData.comicId + scalePostfix +
                               Lightning::FG_BG_IMAGE_URL_POSTFIX;
    unsigned int w, h;
    DataBuffer dataBuffer;
    const char* backgroundPNGPath = "/tmp/background.png";
    HTTPDownloader downloader;
    downloader.downloadFile(fgBgImageUrl, backgroundPNGPath);
    unsigned int error =
        lodepng_decode32_file(&dataBuffer.buffer, &w, &h, backgroundPNGPath);
    assert(error == 0);
    backgroundRgbaPath = "/tmp/background_rgba";
    std::ofstream file(backgroundRgbaPath, std::ios::binary);
    file.write((const char*)dataBuffer.buffer, w * h * 4);
  }

  Lightning::GeneratedSVGs generatedSVGs;
  Lightning::Metrics perfMetrics;
  int curTime = 0;
  for (int f = 0; f < m_arFrame.size(); ++f) {
    Lightning::PerFrameData perFrameData;
    perFrameData.assetLayerStates = std::vector<Lightning::AssetLayerState>(
        3, Lightning::AssetLayerState::ACTIVE);
    perFrameData.jsonPanelData = m_arFrame[f].GenPanelString(strPanelTxt);
    perFrameData.finishTimeMs = curTime;
    perFrameData.comicImagePath = backgroundRgbaPath;
    requestData.comicFrameData.push_back(perFrameData);  //每一帧可能不同
    curTime += 10;
  }
  generatedSVGs = Lightning::generateSVGs(requestData, &perfMetrics);

  // generate webp animation
  std::unique_ptr<std::vector<unsigned char>> webpImageData;
  requestData.imageFormat = Lightning::ImageFormat::ANIMATED_WEBP;
  webpImageData =
      Lightning::svgToCompressedImage(generatedSVGs, requestData, &perfMetrics);
  std::string strWebpPath = strOutputPath + "exported.webp";
  Lightning::saveBytesToFile(strWebpPath, *webpImageData);
  strWebpPath = Lightning::getAbsolutePath(strWebpPath);

  // view webp animation in chrome browser
  std::string strCommand = "open -a \"Google Chrome\" file:";
  strCommand = strCommand + "\"" + strWebpPath + "\"";
  system(strCommand.c_str());
}

-- **************************************************************************
-- structure
-- **************************************************************************
ScriptName = "wangyiMohoExporter"
wangyiMohoExporter = {}

function wangyiMohoExporter:Name()
	return "wangyiMohoExporter"
end

function wangyiMohoExporter:UILabel()
	return "wangyiMohoExporter"
end

function wangyiMohoExporter:Description()
	return "export moho information"
end

function wangyiMohoExporter:Version()
	return "01.00.000"
end

function wangyiMohoExporter:Creator()
	return "GUN"
end

function wangyiMohoExporter:IsBeginnerScript()
	return false 
end

function wangyiMohoExporter:IsEnabled(moho)
	return true
end 

function wangyiMohoExporter:ColorizeIcon()
	return true
end   

wangyiMohoExporter.toPath = ""

-- **************************************************************************
-- UI: let's not using any dialog box
-- ************************************************************************** 

-- **************************************************************************
-- Function Run: The main routine
-- ************************************************************************** 

function wangyiMohoExporter:recursiveIterateLayers(moho, pCurrentLayer, strIndent, arSwitchLayers, arSkeletonLayers, pFile)
	if(pCurrentLayer:LayerType() == MOHO.LT_SWITCH) then
		table.insert(arSwitchLayers, pCurrentLayer)
	end

	if(pCurrentLayer:LayerType() == MOHO.LT_BONE) then
		table.insert(arSkeletonLayers, pCurrentLayer)
	end 

	if(not pCurrentLayer:IsGroupType()) then
		return
	end
 
	local pGroup = moho:LayerAsGroup(pCurrentLayer)
	local dSubLayerCount = pGroup:CountLayers();

	for sl = 0, dSubLayerCount-1,1 do
		local pSubLayer = pGroup:Layer(sl) 
		local strIndentMore = strIndent .. "\t";
		pFile:write(strIndent, tostring(sl), ":", pSubLayer:Name(), "\n"); 
		wangyiMohoExporter:recursiveIterateLayers(moho, pSubLayer, strIndentMore, arSwitchLayers, arSkeletonLayers, pFile);
	end 
end 

function wangyiMohoExporter:exportBoneLayerChannels(moho, pBoneLayer, pFile)
	if(not pBoneLayer:IsBoneType()) then
		return; 
	end 

	local pSkeleton = MOHO.BoneLayer.Skeleton(pBoneLayer);
	local dBoneCount = pSkeleton:CountBones();
	for b=0,dBoneCount-1,1 do
		local pBone = pSkeleton:Bone(b);
		print(pBone:Name(), ".pos:"); pFile:write(pBone:Name(), ".pos:" );  
		print("(-1,", pBone.fPos.x, ",", pBone.fPos.y, ") "); pFile:write("(-1,", pBone.fPos.x, ",", pBone.fPos.y, ") "); 
		local pChannelPos = pBone.fAnimPos;   
		for f = 0,pChannelPos:Duration()-1,1 do
			if pChannelPos:HasKey(f) then
				local pPos = pChannelPos:GetValue(f);
				print(f, ":", pPos.x, ",", pPos.y, " "); pFile:write("(",tostring(f), ",", tostring(pPos.x), ",", tostring(pPos.y), ") ");
			end
		end
		pFile:write("\n"); 

		print(pBone:Name(), ".rot:"); pFile:write(pBone:Name(), ".rot:");
		print("(-1,", pBone.fAngle, ") "); pFile:write("(-1,", pBone.fAngle, ") ");  
		local pChannelAngle = pBone.fAnimAngle; 
		for f = 0,pChannelAngle:Duration()-1,1 do
			if pChannelAngle:HasKey(f) then
				local pAngle = pChannelAngle:GetValue(f);
				pAngle = pAngle*180/math.pi
				print(f, ":", pAngle); pFile:write("(", tostring(f), ",", tostring(pAngle), ") ");
			end
		end 
		pFile:write("\n");
	end  
end

function wangyiMohoExporter:exportSwitchLayerChannels(moho, pSwitchLayer, pFile)
	if(pSwitchLayer:LayerType() ~= MOHO.LT_SWITCH) then
		return; 
	end

	print("SwitchLayer:", pSwitchLayer:Name()); pFile:write(pSwitchLayer:Name(), ":");
	local dChannelCount = pSwitchLayer:CountChannels();
	for c = 0, dChannelCount-1 do
		local pChannelInfo = MOHO.MohoLayerChannel:new_local()
		pSwitchLayer:GetChannelInfo(c, pChannelInfo)  
		local dSubChannelCount = pChannelInfo.subChannelCount 
		if pChannelInfo.channelID == CHANNEL_LAYER_ALL then
			for sc = 0, dSubChannelCount - 1 do
				local pChannel = pSwitchLayer:Channel(c, sc, moho.document)  
				local pChannelWithType;
				if pChannel:ChannelType() == MOHO.CHANNEL_BOOL then
					pChannelWithType = moho:ChannelAsAnimBool(pChannel)
				elseif pChannel:ChannelType() == MOHO.CHANNEL_COLOR then
					pChannelWithType = moho:ChannelAsAnimColor(pChannel)
				elseif pChannel:ChannelType() == MOHO.CHANNEL_STRING then
					pChannelWithType = moho:ChannelAsAnimString(pChannel)
				elseif pChannel:ChannelType() == MOHO.CHANNEL_VAL then
					pChannelWithType = moho:ChannelAsAnimVal(pChannel)
				elseif pChannel:ChannelType() == MOHO.CHANNEL_VEC2 then
					pChannelWithType = moho:ChannelAsAnimVec2(pChannel)
				elseif pChannel:ChannelType() == MOHO.CHANNEL_VEC3 then
					pChannelWithType = moho:ChannelAsAnimVec3(pChannel)
				end 

                for f = 0,pChannel:Duration()-1,1 do
                	if pChannel:HasKey(f) then
                		local value = pChannelWithType:GetValue(f);
                		print(f, ":", value); pFile:write("(", tostring(f), ",", value, ") ");
                	end
                	if f == pChannel:Duration()-1 then
                		pFile:write("\n");
                	end
                end 
            end
        end
    end
end  


-- uncomment, if we need to call lightning directly here
-- require("clibLightning") 

function wangyiMohoExporter:Run(moho) 

    wangyiMohoExporter.toPath = "/Users/Yiyang.Wang/documents/moho/exported/" .. moho.document:Name() .. ".txt";
	local pFile = io.open(wangyiMohoExporter.toPath, "w") 
	if (pFile == nil) then
		LM.GUI.Alert(ALERT_WARNING,"Could not open/write the selected file: ", wangyiMohoExporter.toPath)
		return
	end	

    -- document info: layer, camera --------------------------------------------------------------------------------------------- 
    pFile:write("DOCUMENT INFO:", "\n");
    arSwitchLayers = {};
    arSkeletonLayers = {}; 
	local dLayerCount = moho.document:CountLayers();
	for l = 0,dLayerCount-1,1 do  
		wangyiMohoExporter:recursiveIterateLayers(moho, moho.document:Layer(l), " ", arSwitchLayers, arSkeletonLayers, pFile); 
	end  
	pFile:write("\n")
	pFile:write("LM_ZoomCamera.DEFAULT_FOV:", math.deg(LM_ZoomCamera.DEFAULT_FOV), "\n");
	pFile:write("Dimension:",moho.document:Width(),",",moho.document:Height(),",",moho.document:AspectRatio(),"\n\n\n"); 
	sc = #arSwitchLayers; 
	bc = #arSwitchLayers;
	print("switch layer count:", sc, "bone layer count:", bc)




	-- per frame hard coded ------------------------------------------------------------------------------------------------------
	pFile:write("BY FRAME:", "\n")  
    
	local boneLayer = arSkeletonLayers[1]; 
	local pSkel = MOHO.BoneLayer.Skeleton(boneLayer);--boneLayer:Skeleton() 
	arBoneName = {"B1", "spine 1", "spine 2", "neck", "head", "B21", "bicep R", "fore R", "hand R", 
	"B20",  "bicep L",  "fore L", "hand L", "B19", "thigh R", "shin R", "foot R", "toe R", "thigh L", "shin L", "foot L", "toe L"}; 

	dSwitchLayerCount = #arSwitchLayers; 
	local dDuration = moho.document:AnimDuration();
	for f=1,dDuration,1 do 
		moho:SetCurFrame(f);
		pFile:write("Frame:", tostring(f), "\n");

		-- camera: campos.x,campos.y,campos.z,zoom,roll,tilt,pan
		pFile:write("camera:");
		local campos = moho.document.fCameraTrack:GetValue(f);
		pFile:write(campos.x, ",", campos.y, ",", campos.z, ",");  
		pFile:write(moho.document.fCameraZoom:GetValue(f),",")   
		pFile:write(math.deg(moho.document.fCameraRoll:GetValue(f)), ",") 
		local pt = moho.document.fCameraPanTilt:GetValue(f) 
		pFile:write(math.deg(pt.x), ",", math.deg(pt.y),"\n")  

		-- bone layer translate, scale, rotate, origin
		local t = boneLayer.fTranslation:GetValue(f);
		local rx = math.deg(boneLayer.fRotationX:GetValue(f));
		local ry = math.deg(boneLayer.fRotationY:GetValue(f));
		local rz = math.deg(boneLayer.fRotationZ:GetValue(f));
		local s = boneLayer.fScale:GetValue(f);
		local og = boneLayer:Origin();  
		pFile:write("blayer:", t.x, ",", t.y, ",", t.z, ",", -- bone layer translate
			s.x, ",", s.y, ",", s.z, ",", -- bone layer scale
			rx, ",", ry, ",", rz, ",", -- bone layer rotate
			og.x, ",", og.y, "\n"); -- bone layer origin 

		-- body skeleton
		for b,strBoneName in ipairs(arBoneName) do 
   			local pBone = pSkel:BoneByName(strBoneName)
   			local pos = pBone.fAnimPos:GetValue(f, pBone.fPos)
			local angle = pBone.fAnimAngle:GetValue(f, pBone.fAngle);
			angle = math.deg(angle); 
			pFile:write(strBoneName, ":", tostring(pos.x), ",", tostring(pos.y), ",", tostring(angle), "\n");  
		end

		-- face switch layer
		for sl =1,dSwitchLayerCount,1 do
			local pSwitchLayer = arSwitchLayers[sl]; 
			if(pSwitchLayer:LayerType() == MOHO.LT_SWITCH) then  
				value = MOHO.SwitchLayer.GetValue(pSwitchLayer, f); 
				pFile:write(pSwitchLayer:Name(), ":", value, "\n"); 
			end
		end 
		
		pFile:write("\n");
	end  

	--[[ by channel sparse ------------------------------------------------------------------------------------------------------
	print("BY CHANNEL:") pFile:write("BY CHANNEL:", "\n")  
	-- switch layer channels 
	print("SWITCHLAYER CHANNELS:") pFile:write("SWITCHLAYER CHANNEL:", "\n")   
	for sl =1,dSwitchLayerCount,1 do
		local pSwitchLayer = arSwitchLayers[sl]; 
		wangyiMohoExporter:exportSwitchLayerChannels(moho, pSwitchLayer, pFile); 
	end
	pFile:write("\n");
	-- bone layer channels
	print("BONELAYER CHANNELS:") pFile:write("BONELAYER CHANNEL:", "\n")
	dBoneLayerCount = #arSkeletonLayers;
	for bl = 1,dBoneLayerCount,1 do
		local pBoneLayer = arSkeletonLayers[bl]; 
		wangyiMohoExporter:exportBoneLayerChannels(moho, pBoneLayer, pFile);
	end 
	--]]
	pFile:close()


	-- directly call lightning  --------------------------------------------------------------------------------------------------
	-- print("\ncall lightning:")
	-- wLightning(wangyiMohoExporter.toPath, moho.document:Name(), 1); 
	-- print(moho.document:Name(), ".webp shown in chrome browser");
	
end
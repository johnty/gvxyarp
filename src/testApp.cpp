#include "testApp.h"



string testString;


//--------------------------------------------------------------
void testApp::setup(){

	ofSetCircleResolution(50);
    
	ofSetWindowTitle("openframeworks gvf visualiser");
    ofSetWindowShape(1024, 768);
    
	ofSetFrameRate(60); // if vertical sync is off, we can go a bit fast... this caps the framerate at 60fps.
    
    ofPoint wSize = ofGetWindowSize();
    scrW = wSize.x;
    scrH = wSize.y;
    printf("w: %d h: %d\n", scrW, scrH);
    
    drawArea = ofRectangle(ofPoint(0, 0), ofGetWindowWidth(), ofGetWindowHeight());
    
    currentGesture.setDrawArea(drawArea);
    
    isMouseDrawing = false;
    
    viewYRotation = 0;
    viewXRotation = 0;
    
    initializeGui();
    initColors();
    
    port.open("/ofxgvf");
    active = false;

    
	leap.open();

	cam.setOrientation(ofPoint(-20, 0, 0));

}


//--------------------------------------------------------------
void testApp::update(){

    // if the user is performing a gesture,
    // feed the last point on the gesture to the gvf handler
    // (depending on the speed the user is performing the gesture,
    // the same point might be fed several times)
    //printf("pending reads = %i\n", port.getPendingReads());
    while (port.getPendingReads() != 0) {
        
        yarp::os::Bottle *input = port.read();
        
        if (input->get(0).toString() == "/device") {

            //up or down
            double accel_x = input->get(5).asDouble();
            double accel_y = input->get(6).asDouble();
            double accel_z = input->get(7).asDouble();
            
            double mx = fakeMouseX = accel_x*100.0+50.0;
            double my = fakeMouseY = accel_y*100.0+50.0;
            printf("%f %f\n", fakeMouseX, fakeMouseY);
            
            if (active) {
                mouseDragged(mx, my, 0);
            }

        }
        
    }
    
    
    if(isMouseDrawing)
    {
        gvfh.gvf_data(currentGesture.getLastPointAdded());
    }
    
    
    //leap
    //here we get the points from the leap motion
    //only the position of the first hand is considered:
    //  simpleHands[0].handPos
    //to be continued...
    simpleHands = leap.getSimpleHands();
    if( leap.isFrameNew() && simpleHands.size() )
    {
        ofPoint pt = simpleHands[0].handPos;
    }
    
    
	fingersFound.clear();
	
	//here is a simple example of getting the hands and using them to draw trails from the fingertips.
	//the leap data is delivered in a threaded callback - so it can be easier to work with this copied hand data
	
	//if instead you want to get the data as it comes in then you can inherit ofxLeapMotion and implement the onFrame method.
	//there you can work with the frame data directly.
    
    
    
    //Option 1: Use the simple ofxLeapMotionSimpleHand - this gives you quick access to fingers and palms.
    
    simpleHands = leap.getSimpleHands();
    
    if( leap.isFrameNew() && simpleHands.size() ){
        
        // gvfh.gvf_data(currentGesture.getLastPointAdded());
        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
		leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
        leap.setMappingZ(-150, 150, -200, 200);
        
        for(int i = 0; i < simpleHands.size(); i++){
            
            for(int j = 0; j < simpleHands[i].fingers.size(); j++){
                int id = simpleHands[i].fingers[j].id;
                
                ofPolyline & polyline = fingerTrails[id];
                ofPoint pt = simpleHands[0].fingers[0].pos;
                
                //if the distance between the last point and the current point is too big - lets clear the line
                //this stops us connecting to an old drawing
                if( polyline.size() && (pt-polyline[polyline.size()-1] ).length() > 50 ){
                    polyline.clear();
                }
                
                //add our point to our trail
                polyline.addVertex(pt);
                
                //store fingers seen this frame for drawing
                fingersFound.push_back(id);
            }
        }
    }
    
    //IMPORTANT! - tell ofxLeapMotion that the frame is no longer new.
	leap.markFrameAsOld();
    
}

//--------------------------------------------------------------
void testApp::draw(){
    
    float templatesScale = 0.5f;
    if (active) {
        ofBackgroundGradient(ofColor(120), ofColor(40), OF_GRADIENT_CIRCULAR);
    }
    else {
        ofBackgroundGradient(ofColor(2), ofColor(40), OF_GRADIENT_CIRCULAR);
    }
    ofPushMatrix();
    
    if(rotate)
    {
        // rotate gesture related graphics around the center of the screen
        ofTranslate(scrW/2, scrH/2);
        ofRotateY(viewYRotation);
        ofRotateX(viewXRotation);
        ofTranslate(-scrW/2, -scrH/2);
    }
    
    // draw the current templates on a small scale
    gvfh.drawTemplates(templatesScale);
    
    
    if(gvfh.get_state() != STATE_FOLLOWING && isMouseDrawing)
        currentGesture.draw();
    else if(displayCurrentGesture)
        currentGesture.draw();
    
    ofDisableAlphaBlending();

    // string used to comunicate to the user of possible commands and of the current state of the application

    string state_string;
    state_string.append("'l' to learn a new template\n'c' to clear\n"
                        "numbers 1 to 4 to toggle visual feedback "
                        "(1 - particles; 2 - template; 3 - estimated gesture; 4 - current gesture)"
                        "\nSTATE_LEARINING [");
    
    
	ofSetColor(200);
	//ofDrawBitmapString("ofxLeapMotion - Example App\nLeap Connected? " + ofToString(leap.isConnected()), 20, 20);
    
	cam.begin();
    
	//ofPushMatrix();
    //ofRotate(90, 0, 0, 1);
//    ofSetColor(20);
//    ofDrawGridPlane(800, 20, false);
//	ofPopMatrix();
//	
//	ofEnableLighting();
//	l1.enable();
//	l2.enable();
//	
//	m1.begin();
//	m1.setShininess(0.6);
	
	for(int i = 0; i < fingersFound.size(); i++){
		ofxStrip strip;
		int id = fingersFound[i];
		
		ofPolyline & polyline = fingerTrails[id];
		strip.generate(polyline.getVertices(), 15, ofPoint(0, 0.5, 0.5) );
		
		ofSetColor(255 - id * 15, 0, id * 25);
		strip.getMesh().draw();
	}
//	    for(int i = 0; i < simpleHands.size(); i++){
//        simpleHands[i].debugDraw();
//    }
    
	//m1.end();
	cam.end();

    
    
    int state = gvfh.get_state();
    if(state == STATE_FOLLOWING){
        state_string.append(" ]\nSTATE_FOLLOWING [X]\nSTATE_CLEAR     [ ]");
        
        if(displayParticles)
            gvfh.printParticleInfo(currentGesture);

        // temp will have the partial representation of how gvf is recognising the gesture being performed
        gvfGesture temp = gvfh.getRecognisedGestureRepresentation();
        if(temp.isValid)
        {
            // the estimated gesture will be drawn on the same area
            // as the gesture being performed and starting on the same point
            ofRectangle da = currentGesture.getDrawArea();
            ofPoint p = currentGesture.getInitialOfPoint();
            
            if(displayEstimatedGesture)
            {
                temp.setAppearance(ofColor(0,255,0), 5, 255, 180, 1);
                temp.draw(templatesScale);
                
                temp.setDrawArea(da);
                temp.setInitialPoint(p + 1);
                temp.centraliseDrawing = false;
                temp.draw();
            }
            
            if(displayTemplate)
            {
                // draw the original template for the gesture being recognised
                gvfGesture g = gvfh.getTemplateGesture(gvfh.getIndexMostProbable());
                g.setDrawArea(da);
                g.setInitialPoint(p + 1);
                g.centraliseDrawing = false;

                // the template's opacity is determined by how probable the recognition is
                g.setAppearance(g.getColor(),
                                1.5, 255, 50,
                                ofLerp(1/gvfh.getTemplateCount(),
                                1,
                                gvfh.getRecogInfoOfMostProbable().probability));
               g.draw();
            }
        }
        
    }else if (state == STATE_LEARNING)
        state_string.append("X]\nSTATE_FOLLOWING [ ]\nSTATE_CLEAR     [ ]");
    else
        state_string.append(" ]\nSTATE_FOLLOWING [ ]\nSTATE_CLEAR     [X]");
    
    ofPopMatrix();
    
    ofSetColor(198);
    
    ofDrawBitmapString(state_string.c_str(), 30, 25);

    // show the current frame rate
    ofDrawBitmapString("FPS " + ofToString(ofGetFrameRate(), 0), ofGetWidth() - 200, 25);
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){

	if (key == 'l' || key == 'L'){
        // get ready to learn a new gesture
        // (will not start to learn a new gesture
        // if the user is in the middle of a gesture
        // or if the state is already STATE_LEARNING)
		if(gvfh.get_state() != STATE_LEARNING && !isMouseDrawing)
        {
            gvfh.gvf_learn();
        }
	}
    else if(key == 'c' || key == 'C')
    {
        gvfh.gvf_clear();
        initColors();
    }
    else if (key == 'r' || key == 'R')
    {
        rotate = !rotate;
        viewYRotation = 0;
        viewXRotation = 0;
    }
    else if (key == 'f' || key == 'F')
    {
        ofToggleFullscreen();
    }
    else if (key == '1')
    {
        displayParticles = !displayParticles;
    }
    else if (key == '2')
    {
        displayTemplate = !displayTemplate;
    }
    else if (key == '3')
    {
        displayEstimatedGesture = !displayEstimatedGesture;
    }
    else if (key == '4')
    {
        displayCurrentGesture = !displayCurrentGesture;
    }
    else if (key == ' ') {
        //dummy "mouse down"
        if (active) {
            printf("mUP");
            mouseReleased(fakeMouseX, fakeMouseY, 0);
            active = false;
        }
        else {
            printf("mDown\n");
            mousePressed(fakeMouseX, fakeMouseY, 0);
            active = true;
        }
        
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
    // if rotating, the mouse will determing how many degrees
    if(rotate)
    {
        viewYRotation = ofMap(x, 0, ofGetWidth(), -90, 90);
        viewXRotation = ofMap(y, 0, ofGetHeight(), 90, -90);
    }
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
    
    // if a gesture has already been starded, a new point is added to it
    if(isMouseDrawing)
    {
        currentGesture.addNonNormalisedPoint(x, y);
    }
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

    // a new gesture will not start if the user clicks on the UI area or outside the gesture area
    if(currentGesture.isPointInGestureArea(x, y) && !currentGesture.isPointInArea(guiArea, x, y))
    {
        // the current gesture is initialised with its initial point
        currentGesture.initialiseNonNormalised(x, y);
        
        // here the point is already normalised
        ofPoint initialPoint = currentGesture.getInitialOfPoint();
        
        if(gvfh.get_state() == STATE_LEARNING)
        {
            gvfh.addTemplateGesture(initialPoint, generateRandomColor());
        }

        // first point is always 0.5 (the gesture's initialPoint property can be used to translate the gesture)
        currentGesture.addPoint(ofPoint(0.5, 0.5));

        isMouseDrawing = true;   

    }
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    if(isMouseDrawing)
    {
        isMouseDrawing = false;
    }
    
    // if a gesture has just been learnt, automatically switches the state to following
    if(gvfh.get_state() == STATE_LEARNING)
        gvfh.gvf_follow();
    
    if(gvfh.get_state() == STATE_FOLLOWING)
        gvfh.gvf_restart();
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    ofPoint wSize = ofGetWindowSize();
    scrW = wSize.x;
    scrH = wSize.y;
    printf("w: %d h: %d\n", scrW, scrH);
    
    // resets the current gesture's draw area with the new size
    drawArea = ofRectangle(ofPoint(0, 0), ofGetWindowWidth(), ofGetWindowHeight());

    currentGesture.setDrawArea(drawArea);
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
    
}

void testApp::initializeGui()
{
    float guiWidth = 200;
    float guiHeight = 500;
    guiArea = ofRectangle(0, 100, guiWidth, guiHeight);
    settingsGui = new ofxUICanvas(guiArea);
    
    // the initial parameters values set here are not connected to the initial parameters values on the gvfhandler constructor
    // those are the real initial values. The values here will only take effect once the user changes them
    // (this is inconsistent and should be fixed)
    
    settingsGui->addWidgetDown(new ofxUILabel("Amount of particles", OFX_UI_FONT_MEDIUM));
    nsNumDialer = new ofxUINumberDialer(10, 10000, 2000, 0, "amount of particles", OFX_UI_FONT_MEDIUM);
    settingsGui->addWidgetDown(nsNumDialer);
    settingsGui->addSpacer(guiWidth - 2, 2);
    
    settingsGui->addWidgetDown(new ofxUILabel("Resampling Threshold", OFX_UI_FONT_MEDIUM));
    rtNumDialer = new ofxUINumberDialer(100, 10000, 1000, 0, "resampling threshold", OFX_UI_FONT_MEDIUM);
    settingsGui->addWidgetDown(rtNumDialer);
    settingsGui->addSpacer(guiWidth - 2, 2);
    
    settingsGui->addWidgetDown(new ofxUILabel("Smoothing Coefficient", OFX_UI_FONT_MEDIUM));
    soNumDialer = new ofxUINumberDialer(0.01, 2, 0.2, 2, "smoothing coefficient", OFX_UI_FONT_MEDIUM);
    settingsGui->addWidgetDown(soNumDialer);
    settingsGui->addSpacer(guiWidth - 2, 2);
    
    settingsGui->addWidgetDown(new ofxUILabel("", OFX_UI_FONT_MEDIUM));
    settingsGui->addWidgetDown(new ofxUILabel("Variance coefficients", OFX_UI_FONT_MEDIUM));
    
    settingsGui->addWidgetDown(new ofxUILabel("Position", OFX_UI_FONT_MEDIUM));
    sigPosND = new ofxUINumberDialer(0.000001, 0.1, 0.0001, 6, "position", OFX_UI_FONT_MEDIUM);
    settingsGui->addWidgetDown(sigPosND);
    
    settingsGui->addWidgetDown(new ofxUILabel("Speed", OFX_UI_FONT_MEDIUM));
    sigSpeedND = new ofxUINumberDialer(0.000001, 0.1, 0.01, 6, "speed", OFX_UI_FONT_MEDIUM);
    settingsGui->addWidgetDown(sigSpeedND);
    
    settingsGui->addWidgetDown(new ofxUILabel("Scale", OFX_UI_FONT_MEDIUM));
    sigScaleND = new ofxUINumberDialer(0.000001, 0.1, 0.0001, 6, "scale", OFX_UI_FONT_MEDIUM);
    settingsGui->addWidgetDown(sigScaleND);
    
    settingsGui->addWidgetDown(new ofxUILabel("Rotation", OFX_UI_FONT_MEDIUM));
    sigRotND = new ofxUINumberDialer(0.000001, 0.1, 0.000001, 6, "rotation", OFX_UI_FONT_MEDIUM);
    settingsGui->addWidgetDown(sigRotND);
    
    settingsGui->addWidgetDown(new ofxUILabel("", OFX_UI_FONT_MEDIUM));
    settingsGui->addLabelButton("Save gesture(s)", false);
    settingsGui->addLabelButton("Load gesture(s)", false);
    
    ofAddListener(settingsGui->newGUIEvent, this, &testApp::guiEvent);
}

void testApp::guiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();
    
	cout << "got event from: " << name << endl;
	
    // if any paramenter needs to be adjusted,
    // the appropriate gvfhandler method will be called
    
    if(name == "amount of particles")
	{
        cout << nsNumDialer->getValue() << endl;
        gvfh.setNumberOfParticles(nsNumDialer->getValue());
    }
    else if(name == "resampling threshold")
    {
        gvfh.gvf_rt((int) rtNumDialer->getValue());
    }
    else if(name == "smoothing coefficient")
    {
        gvfh.gvf_std(soNumDialer->getValue());
    }
    else if(name == "position" || name == "speed" ||
            name == "scale" || name == "rotation")
    {
        std::vector<float> sigs;
        sigs.push_back(sigPosND->getValue());
        sigs.push_back(sigSpeedND->getValue());
        sigs.push_back(sigScaleND->getValue());
        sigs.push_back(sigRotND->getValue());
        
        gvfh.gvf_adaptspeed(sigs);
    }
    
    // if save or load is requested,
    // the appropriate dialog is shown and the task is carried out
    else if(name == "Save gesture(s)")
    {
        ofxUILabelButton *button = (ofxUILabelButton*) e.widget;
        if(button->getValue() && gvfh.getTemplateCount() > 0)
        {
            ofFileDialogResult dialogResult = ofSystemSaveDialog("my gestures.xml", "Save gestures");
            if(dialogResult.bSuccess)
            {
                saveGestures(dialogResult);
            }
        }
    }
    else if(name == "Load gesture(s)")
    {
        ofxUILabelButton *button = (ofxUILabelButton*) e.widget;
        if(button->getValue())
        {
            ofFileDialogResult dialogResult = ofSystemLoadDialog("Select the xml file containing gesture data");
            if(dialogResult.bSuccess)
            {
                loadGestures(dialogResult);
            }
            
        }
    }
}

void testApp::loadGestures(ofFileDialogResult dialogResult)
{
    ofxXmlSettings file;
    if(!file.loadFile(dialogResult.filePath))
        return;
    
    gvfh.gvf_clear();
    initColors();
    
    int gestureCount = file.getNumTags("GESTURE");
    if(gestureCount < 1)
        return;
    
    cout << gestureCount << " gestures." << endl;
    
    for(int i = 0; i < gestureCount; i++)
    {
        file.pushTag("GESTURE", i);

        ofPoint p;
        p.x = file.getValue("INIT_POINT:X", (double)-1);
        p.y = file.getValue("INIT_POINT:Y", (double)-1);
        
        file.pushTag("POINTS");
            int pointCount = file.getNumTags("PT");
            if(pointCount < 1)
                return;

            gvfh.gvf_learn();        
            gvfh.addTemplateGesture(p, generateRandomColor());
        
        for(int j = 0; j < pointCount; j++)
        {
            p.x = file.getValue("PT:X", (double)-1, j);
            p.y = file.getValue("PT:Y", (double)-1, j);
            gvfh.gvf_data(p);
        }
        file.popTag();
        file.popTag();
        gvfh.gvf_follow();
    }
}

void testApp::saveGestures(ofFileDialogResult dialogResult)
{
    ofxXmlSettings file;
    cout << dialogResult.filePath << endl;
    cout << dialogResult.fileName << endl;
    int templateCount = gvfh.getTemplateCount();
    for(int i = 0; i < templateCount; i++)
    {
        gvfGesture g = gvfh.getTemplateGesture(i);
        ofPoint initialPoint = g.getInitialOfPoint();
        int currentGesture = file.addTag("GESTURE");
        file.pushTag("GESTURE", currentGesture);
        file.addTag("INIT_POINT");
        file.pushTag("INIT_POINT");
        file.addValue("X", initialPoint.x);
        file.addValue("Y", initialPoint.y);
        file.popTag();
        file.addTag("POINTS");
        file.pushTag("POINTS");
        std::vector<std::vector<float> > allPoints = g.getData();
        int pointCount = allPoints.size();
        for(int j = 0; j < pointCount; j++)
        {
            int tagNum = file.addTag("PT");
            file.setValue("PT:X", allPoints[j][0], tagNum);
            file.setValue("PT:Y", allPoints[j][1], tagNum);
        }
        file.popTag();
        file.popTag();
    }
    file.saveFile(dialogResult.filePath);
}

void testApp::initColors()
{
    colors.clear();
    colors.push_back(ofColor::white);
    colors.push_back(ofColor::gray);
    colors.push_back(ofColor::blue);
    colors.push_back(ofColor::cyan);
    colors.push_back(ofColor::olive);
    colors.push_back(ofColor::gold);
    colors.push_back(ofColor::magenta);
    colors.push_back(ofColor::violet);
}

ofColor testApp::generateRandomColor()
{
    ofColor c;
    
    if(colors.size() == 0)
        initColors();
    
    int colorsRemaining = colors.size();
    
    int index = ofRandom(0, colorsRemaining - 1);
    cout << index << endl;
    c = colors[index];
    colors.erase(colors.begin() + index);
    return c;
}

//--------------------------------------------------------------
void testApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
    port.close();
}



#include "ANFScene.h"
#include <string>
#include <iostream>
#include "CGFaxis.h"
#include "CGFapplication.h"
#include <math.h>

using namespace std;


float pi = acos(-1.0);
float deg2rad=pi/180.0;

ANFScene::ANFScene(char *filename)
{
	// Read ANF  from file

	doc=new TiXmlDocument( filename );
	bool loadOkay = doc->LoadFile();

	if ( !loadOkay )
	{
		printf( "Could not load file '%s'. Error='%s'. Exiting.\n", filename, doc->ErrorDesc() );

		exit( 1 );
	}

	TiXmlElement* anfElement= doc->FirstChildElement( "anf" );

	if (anfElement == NULL)
	{
		printf("Main anf block element not found! Exiting!\n");system("pause");	
		exit(1);
	}

	parser;

	globalsElement = anfElement->FirstChildElement( "globals" );
	lightsElement = anfElement->FirstChildElement( "lights" );
	camerasElement = anfElement->FirstChildElement( "cameras" );
	textureElement = anfElement->FirstChildElement( "textures" );
	appearancesElement= anfElement->FirstChildElement("appearances");
	graphElement = anfElement->FirstChildElement("graph");


	//Globals

	if (globalsElement == NULL)
		printf("Globals block not found!\n");
	else
	{
		printf("Processing globals:\n");

		TiXmlElement* drawingElement=globalsElement->FirstChildElement("drawing");
		if (drawingElement)
		{

			char *valString=NULL;
			float b1,b2,b3,b4;

			parser.globals->drawing.mode = drawingElement->Attribute("mode");
			parser.globals->drawing.shading = drawingElement->Attribute("shading");


			valString=(char *) drawingElement->Attribute("background");

			if(valString && sscanf(valString,"%f %f %f %f",&b1, &b2, &b3, &b4)==4)
			{
				parser.globals->drawing.background[0] = b1;
				parser.globals->drawing.background[1] = b2;
				parser.globals->drawing.background[2] = b3;
				parser.globals->drawing.background[3] = b4;
			}
			else
				printf("Error parsing background");
		}
		else
			printf("drawing not found\n");	

		TiXmlElement* cullingElement=globalsElement->FirstChildElement("culling");
		if (cullingElement)
		{
			parser.globals->culling.face = cullingElement->Attribute("face");
			parser.globals->culling.order = cullingElement->Attribute("order");
		}
		else
			printf("culling not found\n");	

		TiXmlElement* lightingElement=globalsElement->FirstChildElement("lighting");
		if (lightingElement)
		{

			parser.globals->lighting.local = lightingElement->Attribute("local");
			parser.globals->lighting.doublesdd = lightingElement->Attribute("doublesided");
			parser.globals->lighting.enabled = lightingElement->Attribute("enabled");

			char *valString=NULL;
			float b1,b2,b3,b4;

			valString=(char *) lightingElement->Attribute("ambient");

			if(valString && sscanf(valString,"%f %f %f %f",&b1, &b2, &b3, &b4)==4)
			{
				parser.globals->lighting.ambient[0] = b1;
				parser.globals->lighting.ambient[1] = b2;
				parser.globals->lighting.ambient[2] = b3;
				parser.globals->lighting.ambient[3] = b4;
			}
			else
				printf("Error parsing ambient");
		}
		else
			printf("lighting not found\n");	

	}

	//Cameras
	if (camerasElement == NULL)
		printf("Cameras block not found!\n");
	else
	{
		printf("Processing camerass:\n");

		TiXmlElement* perspective=camerasElement->FirstChildElement("perspective");
		CPerspective cPer;
		TiXmlElement* ortho=camerasElement->FirstChildElement("ortho");
		COrtho cOrt;

		parser.initCam = camerasElement->Attribute("initial");
		parser.activeCam = parser.initCam;

		while(perspective){
			cPer.id = perspective->Attribute("id");

			float near, far, angle, posx, posy, posz, targetx, targety, targetz;
			char *pos=NULL, *target=NULL;

			if(perspective->QueryFloatAttribute("near",&near)==TIXML_SUCCESS)
				cPer.near = near;
			if(perspective->QueryFloatAttribute("far",&far)==TIXML_SUCCESS)
				cPer.far = far;
			if(perspective->QueryFloatAttribute("angle",&angle)==TIXML_SUCCESS)
				cPer.angle = angle;

			pos=(char *) perspective->Attribute("pos");

			if(pos && sscanf(pos,"%f %f %f",&posx, &posy, &posz)==3)
			{
				cPer.pos[0] = posx;
				cPer.pos[1] = posy;
				cPer.pos[2] = posz;
			}
			else
				printf("\tError reading position\n");
			target=(char *) perspective->Attribute("target");

			if(target && sscanf(target,"%f %f %f",&targetx, &targety, &targetz)==3)
			{
				cPer.target[0] = targetx;
				cPer.target[1] = targety;
				cPer.target[2] = targetz;
			}
			else
				printf("\tError reading target\n");

			parser.cameras[cPer.id] = cPer;
			perspective=perspective->NextSiblingElement("perspective");
		}



		float direction,near1,far1,left,right,top,bottom;
		while(ortho)
		{
			cOrt.id = ortho->Attribute("id");

			if(ortho->QueryFloatAttribute("direction",&direction)==TIXML_SUCCESS)
				cOrt.direction = direction;

			if(ortho->QueryFloatAttribute("near",&near1)==TIXML_SUCCESS)
				cOrt.near = near1;

			if(ortho->QueryFloatAttribute("far",&far1)==TIXML_SUCCESS)
				cOrt.far = far1;

			if(ortho->QueryFloatAttribute("left",&left)==TIXML_SUCCESS)
				cOrt.left = left;

			if(ortho->QueryFloatAttribute("right",&right)==TIXML_SUCCESS)
				cOrt.right = right;

			if(ortho->QueryFloatAttribute("top",&top)==TIXML_SUCCESS)
				cOrt.top = top;

			if(ortho->QueryFloatAttribute("bottom",&bottom)==TIXML_SUCCESS)
				cOrt.bottom = bottom;

			parser.cameras[cOrt.id] = cOrt;
			ortho=ortho->NextSiblingElement("ortho");
		}
	}


	//Lights

	if (lightsElement == NULL)
		printf("Lights block not found!\n");
	else
	{
		printf("Processing Lights:\n");
		TiXmlElement *light=lightsElement->FirstChildElement();

		while (light)
		{
			Light lt;
			lt.id = light->Attribute("id");
			lt.type = light->Attribute("type");
			lt.enabled =light->Attribute("enabled");
			lt.marker = light->Attribute("marker");
			if(lt.type == "spot")
			{
				float angle, exponent,targetx, targety, targetz;
				char *target=NULL;
				if(light->QueryFloatAttribute("angle",&angle)==TIXML_SUCCESS)
					lt.angle = angle;

				if(light->QueryFloatAttribute("exponent",&exponent)==TIXML_SUCCESS)
					lt.exponent = exponent;

				target=(char *) light->Attribute("target");

				if(target && sscanf(target,"%f %f %f",&targetx, &targety, &targetz)==3)
				{
					lt.target[0] = targetx;
					lt.target[1] = targety;
					lt.target[2] = targetz;
				}
				else
					printf("\tError reading target\n");
			}

			char *valString=NULL;
			float b1,b2,b3,b4;
			TiXmlElement *component = light->FirstChildElement();

			valString=(char *) component->Attribute("value");

			if(valString && sscanf(valString,"%f %f %f %f",&b1, &b2, &b3, &b4)==4)
			{
				lt.ambient[0] = b1;
				lt.ambient[1] = b2;
				lt.ambient[2] = b3;
				lt.ambient[3] = b4;
			}
			else
				printf("Error parsing ambient\n");

			component= component->NextSiblingElement();

			valString=(char *) component->Attribute("value");

			if(valString && sscanf(valString,"%f %f %f %f",&b1, &b2, &b3, &b4)==4)
			{
				lt.diffuse[0] = b1;
				lt.diffuse[1] = b2;
				lt.diffuse[2] = b3;
				lt.diffuse[3] = b4;
			}
			else
				printf("Error parsing diffuse\n");

			component = component->NextSiblingElement();

			valString=(char *) component->Attribute("value");

			if(valString && sscanf(valString,"%f %f %f %f",&b1, &b2, &b3, &b4)==4)
			{
				lt.specular[0] = b1;
				lt.specular[1] = b2;
				lt.specular[2] = b3;
				lt.specular[3] = b4;
			}
			else
				printf("Error parsing specular\n");


			parser.lights.push_back(&lt);
			light=light->NextSiblingElement();

		}
	}

	//appearances
	float shininess;
	if (appearancesElement==NULL)
		printf("Appearances Block not found!\n");
	else
	{
		TiXmlElement *appearance=appearancesElement->FirstChildElement();
		while (appearance)
		{
			printf("Appearance - %s\n", appearance->Attribute("id"));
			appearance->QueryFloatAttribute("shininess",&shininess);
			printf("\tShininess: %f\n", shininess);
			printf("\tTextureRef: %s\n", appearance->Attribute("textureref"));

			appearance=appearance->NextSiblingElement();
		}
	}

	//Textures

	if(textureElement == NULL)
		printf("Textures block not found!\n");
	else
	{
		TiXmlElement *texture = textureElement->FirstChildElement();
		while(texture)
		{
			printf("Texture - %s\n", texture->Attribute("id"));
			printf("\tfile: %s\n",texture->Attribute("file"));

			texture = texture->NextSiblingElement();
		}
	}

	//Graph

	if (graphElement == NULL)
		printf("Graph block not found!\n");
	else
	{

		TiXmlElement *node=graphElement->FirstChildElement();
		parser.graph->rootID = graphElement->Attribute("rootid");

		if(	node == NULL)
			printf("Node block not found!\n");
		else
		{
			while (node)
			{
				Node* pNode = new Node();
				pNode->id = node->Attribute("id");
				TiXmlElement *transforms = node->FirstChildElement("transforms");
				glLoadIdentity();
				glGetFloatv(GL_MODELVIEW_MATRIX,pNode->matrix);
				if(transforms == NULL)
				{
					printf("Transforms block not found!\n");
					break;
				}
				else
				{
					TiXmlElement *transform = transforms->FirstChildElement("transform");
					while(transform)
					{
						if((string) transform->Attribute("type") == "translate")
						{

							char *pos=NULL;
							float posx,posy,posz;
							pos=(char *) transform->Attribute("to");

							if(pos && sscanf(pos,"%f %f %f",&posx, &posy, &posz)==3)
							{
								glTranslatef(posx,posy,posz);
							}
							else
								printf("\tError reading position\n");
						}
						if((string) transform->Attribute("type") == "rotate")
						{
							string axis = (char*) transform->Attribute("axis");
							float angle;
							if(transform->QueryFloatAttribute("angle",&angle)==TIXML_SUCCESS)
								glRotatef(angle, axis == "x", axis == "y", axis == "z");
						}

						if((string) transform->Attribute("type") == "scale")
						{
							char *pos=NULL;
							float posx,posy,posz;
							pos=(char *) transform->Attribute("factor");

							if(pos && sscanf(pos,"%f %f %f",&posx, &posy, &posz)==3)
							{
								glScalef(posx,posy,posz);
							}
							else
								printf("\tError reading factor\n");
						}


						glGetFloatv(GL_MODELVIEW_MATRIX,pNode->matrix);
						transform = transform->NextSiblingElement();
					}
				}

				TiXmlElement *appearance = node->FirstChildElement("appearanceref");
				if(appearance->Attribute("id") != "inherit")
				{
					pNode->apperance = &(parser.appearances[appearance->Attribute("id")]);
				}

				TiXmlElement *primitives = node->FirstChildElement("primitives");
				if(primitives == NULL)
				{
					printf("Primitives block not found!\n");
					break;
				}
				else
				{

					TiXmlElement *rectangle = primitives->FirstChildElement("rectangle");
					TiXmlElement *triangle = primitives->FirstChildElement("triangle");
					TiXmlElement *cylinder = primitives->FirstChildElement("cylinder");
					TiXmlElement *sphere = primitives->FirstChildElement("sphere");
					TiXmlElement *torus = primitives->FirstChildElement("torus");

					while(rectangle){
						Rectangle* rect = new Rectangle();
						rect->name = "rectangle";

						char *pos=NULL;
						float pos1,pos2;
						pos=(char *) rectangle->Attribute("xy1");

						if(pos && sscanf(pos,"%f %f",&pos1, &pos2)==2)
						{
							rect->xy1[0] = pos1;
							rect->xy1[1] = pos2;
						}
						else
							printf("\tError reading rectangle\n");

						pos=(char *) rectangle->Attribute("xy2");

						if(pos && sscanf(pos,"%f %f",&pos1, &pos2)==2)
						{
							rect->xy2[0] = pos1;
							rect->xy2[1] = pos2;
						}
						else
							printf("\tError reading rectangle\n");

						pNode->primitives.push_back(rect);
						rectangle=rectangle->NextSiblingElement("rectangle");
					}
					while(triangle){

						Triangle* tri = new Triangle();
						tri->name = "triangle";

						char *pos=NULL;
						float pos1,pos2, pos3;
						pos=(char *) triangle->Attribute("xyz1");

						if(pos && sscanf(pos,"%f %f %f",&pos1, &pos2, &pos3)==3)
						{
							tri->xyz1[0] = pos1;
							tri->xyz1[1] = pos2;
							tri->xyz1[2] = pos3;
						}
						else
							printf("\tError reading triangle\n");

						pos=(char *) triangle->Attribute("xyz2");

						if(pos && sscanf(pos,"%f %f %f",&pos1, &pos2, &pos3)==3)
						{
							tri->xyz2[0] = pos1;
							tri->xyz2[1] = pos2;
							tri->xyz2[2] = pos3;
						}
						else
							printf("\tError reading triangle\n");

						pos=(char *) triangle->Attribute("xyz3");

						if(pos && sscanf(pos,"%f %f %f",&pos1, &pos2, &pos3)==3)
						{
							tri->xyz3[0] = pos1;
							tri->xyz3[1] = pos2;
							tri->xyz3[2] = pos3;
						}
						else
							printf("\tError reading triangle\n");

						pNode->primitives.push_back(tri);

						triangle=triangle->NextSiblingElement("triangle");
					}
					while(cylinder){
						Cylinder* cyl = new Cylinder(); 
						cyl->name = "cylinder";

						float base,top,height;
						int slices, stacks;

						if(cylinder->QueryFloatAttribute("base",&base)==TIXML_SUCCESS)
							cyl->base = base;
						if(cylinder->QueryFloatAttribute("top",&top)==TIXML_SUCCESS)
							cyl->top = top;
						if(cylinder->QueryFloatAttribute("height",&height)==TIXML_SUCCESS)
							cyl->height = height;
						if(cylinder->QueryIntAttribute("slices",&slices)==TIXML_SUCCESS)
							cyl->slices = slices;
						if(cylinder->QueryIntAttribute("stacks",&stacks)==TIXML_SUCCESS)
							cyl->stacks = stacks;

						pNode->primitives.push_back(cyl);
						cylinder=cylinder->NextSiblingElement("cylinder");
					}
					while(sphere){
						Sphere* sph = new Sphere();
						sph->name = "sphere";

						float radius;
						int slices, stacks;

						if(sphere->QueryFloatAttribute("radius",&radius)==TIXML_SUCCESS)
							sph->radius = radius;
						if(sphere->QueryIntAttribute("slices",&slices)==TIXML_SUCCESS)
							sph->slices = slices;
						if(sphere->QueryIntAttribute("stacks",&stacks)==TIXML_SUCCESS)
							sph->stacks = stacks;

						pNode->primitives.push_back(sph);
						sphere=sphere->NextSiblingElement("sphere");
					}
					while(torus){

						Torus* tor = new Torus();
						tor->name = "torus";

						float inner, outer;
						int slices, loops;

						if(torus->QueryFloatAttribute("inner",&inner)==TIXML_SUCCESS)
							tor->inner = inner;
						if(torus->QueryFloatAttribute("outer",&outer)==TIXML_SUCCESS)
							tor->outer = outer;
						if(torus->QueryIntAttribute("slices",&slices)==TIXML_SUCCESS)
							tor->slices = slices;
						if(torus->QueryIntAttribute("loops",&loops)==TIXML_SUCCESS)
							tor->loops = loops;

						pNode->primitives.push_back(tor);
						torus=torus->NextSiblingElement("torus");
					}

				}

				TiXmlElement *descendants = node->FirstChildElement("descendants");
				if(descendants !=NULL)
				{
					TiXmlElement *nodeRef = descendants->FirstChildElement();
					while(nodeRef)
					{
						pNode->descendants.push_back(nodeRef->Attribute("id"));

						nodeRef = nodeRef->NextSiblingElement();
					}
				}
				parser.graph->nodes[pNode->id] = pNode;
				node = node->NextSiblingElement();

			}
		}
	}


}

ANFScene::~ANFScene()
{
	delete(doc);
}

//-------------------------------------------------------


void ANFScene::init() 
{

	float globalAmbientLight[4];
	for(int i = 0; i < 4 ;i++)
		globalAmbientLight[i] = parser.globals->lighting.ambient[i];

	if(parser.globals->lighting.enabled)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	if(parser.globals->lighting.doublesdd)
		glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	else
		glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

	if(parser.globals->lighting.local)
		glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	else
		glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);  

	if(parser.globals->drawing.shading == "flat")
		glShadeModel(GL_FLAT);
	else
		glShadeModel(GL_SMOOTH);



	if(parser.globals->drawing.mode == "fill")
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else if(parser.globals->drawing.mode == "point")
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	glClearColor(parser.globals->drawing.background[0], parser.globals->drawing.background[1],
		parser.globals->drawing.background[2],parser.globals->drawing.background[3]);

	//Globals culling
	glEnable(GL_CULL_FACE);

	if(parser.globals->culling.face == "none")
		glDisable(GL_CULL_FACE);
	else if (parser.globals->culling.face == "front")
		glCullFace(GL_FRONT);
	else if (parser.globals->culling.face == "back")
		glCullFace(GL_BACK);
	else
		glCullFace(GL_FRONT_AND_BACK);

	if(parser.globals->culling.order == "cw")
		glFrontFace(GL_CW);
	else
		glFrontFace(GL_CCW);


	// Declares and enables two lights, with null ambient component



	/*light0 = new CGFlight(GL_LIGHT0, light0_pos);
	light0->setAmbient(ambientNull);
	light0->setSpecular(yellow);


	light1 = new CGFlight(GL_LIGHT1, light1_pos);
	light1->setAmbient(ambientNull);


	light2 = new CGFlight(GL_LIGHT2, light2_pos);
	light2->setAmbient(ambientNull);
	light2->setKc(0);
	light2->setKl(1);
	light2->setKq(0);*/




	// Uncomment below to enable normalization of lighting normal vectors
	glEnable (GL_NORMALIZE);
	glEnable (GL_TEXTURE_2D);

	//Declares scene elements

}




void ANFScene::display() 
{

	// ---- BEGIN Background, camera and axis setup

	// Clear image and depth buffer everytime we update the scene
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// Initialize Model-View matrix as identity (no transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// Apply transformations corresponding to the camera position relative to the origin
	CGFscene::activeCamera->applyView();

	/*
	light0->draw();
	light0->disable();
	if (l0 != 0)
	light0->enable();*/


	// Draw axis
	axis.draw();

	// ---- END Background, camera and axis setup

	// ---- BEGIN Primitive drawing section

	drawGraph(parser.graph->rootID);



	// ---- END Primitive drawing section


	// We have been drawing in a memory area that is not visible - the back buffer, 
	// while the graphics card is showing the contents of another buffer - the front buffer
	// glutSwapBuffers() will swap pointers so that the back buffer becomes the front buffer and vice-versa
	glutSwapBuffers();
}

void ANFScene::drawGraph(string nodeID)
{
	Node Cnode;
	Cnode = *parser.graph->nodes[nodeID];
	for(int i = 0; i < Cnode.primitives.size(); i++)
		(*Cnode.primitives[i]).draw();
	
}

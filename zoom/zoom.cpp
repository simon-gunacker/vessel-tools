// Select a point (x,y,z) and find all vessels with at least one node falling
// within a radius rzoom of this point.
// Working with the Amira spatialgraph file format.

#include <cstdio>
#include <vector>

#include <algorithm>
#include <math.h>
#include <string.h>
#include <string>
#include <sstream>
#include <assert.h>
#include <ctime>

#include "network.h"

int WriteCmguiData(char *basename, NETWORK *net, float origin_shift[]);

#define STR_LEN 128

FILE *fperr, *fpout;

//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
float dist(NETWORK *net, int k1, int k2)
{
	float dx = net->point[k2].x - net->point[k1].x;
	float dy = net->point[k2].y - net->point[k1].y;
	float dz = net->point[k2].z - net->point[k1].z;
	return sqrt(dx*dx+dy*dy+dz*dz);
}

//-----------------------------------------------------------------------------------------------------
// This code is faulty - because points can appear multiple times, there are multiple subtractions.
//-----------------------------------------------------------------------------------------------------
int ShiftOrigin(NETWORK *net, float origin_shift[])
{
	int i, k, j;
	EDGE edge;

	for (i=0;i<net->nv;i++) {
		net->vertex[i].point.x -= origin_shift[0];
		net->vertex[i].point.y -= origin_shift[1];
		net->vertex[i].point.z -= origin_shift[2];
	}
	for (i=0;i<net->ne;i++) {
		edge = net->edgeList[i];
		for (k=0;k<edge.npts;k++) {
			j = edge.pt[k];
			net->point[j].x -= origin_shift[0];
			net->point[j].y -= origin_shift[1];
			net->point[j].z -= origin_shift[2];
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------------------------------
// Write Amira SpatialGraph file
//-----------------------------------------------------------------------------------------------------
int WriteAmiraFile(char *amFileOut, char *amFileIn, NETWORK *net, float origin_shift[])
{
	int i, k, j, npts;
	EDGE edge;

	fprintf(fpout,"\nWriteAmiraFile: %s\n",amFileOut);
	npts = 0;
	for (i=0;i<net->ne;i++) {
		npts += net->edgeList[i].npts;
	}

	FILE *fpam = fopen(amFileOut,"w");
	fprintf(fpam,"# AmiraMesh 3D ASCII 2.0\n");
	fprintf(fpam,"# Created by zoom.exe from: %s\n",amFileIn);
	fprintf(fpam,"\n");
	fprintf(fpam,"define VERTEX %d\n",net->nv);
	fprintf(fpam,"define EDGE %d\n",net->ne);
	fprintf(fpam,"define POINT %d\n",npts);
	fprintf(fpam,"\n");
	fprintf(fpam,"Parameters {\n");
	fprintf(fpam,"    ContentType \"HxSpatialGraph\"\n");
	fprintf(fpam,"}\n");
	fprintf(fpam,"\n");
	fprintf(fpam,"VERTEX { float[3] VertexCoordinates } @1\n");
	fprintf(fpam,"EDGE { int[2] EdgeConnectivity } @2\n");
	fprintf(fpam,"EDGE { int NumEdgePoints } @3\n");
	fprintf(fpam,"POINT { float[3] EdgePointCoordinates } @4\n");
	fprintf(fpam,"POINT { float thickness } @5\n");
	fprintf(fpam,"\n");
	fprintf(fpam,"\n@1\n");

	for (i=0;i<net->nv;i++) {
//		fprintf(fpam,"%6.1f %6.1f %6.1f\n",net->vertex[i].point.x,net->vertex[i].point.y,net->vertex[i].point.z);
		fprintf(fpam,"%6.1f %6.1f %6.1f\n",
			net->vertex[i].point.x - origin_shift[0],
			net->vertex[i].point.y - origin_shift[1],
			net->vertex[i].point.z - origin_shift[2]);
	}
	fprintf(fpam,"\n@2\n");
	for (i=0;i<net->ne;i++) {
		edge = net->edgeList[i];
		fprintf(fpam,"%d %d\n",edge.vert[0],edge.vert[1]);
	}
	fprintf(fpam,"\n@3\n");
	for (i=0;i<net->ne;i++) {
		edge = net->edgeList[i];
		fprintf(fpam,"%d\n",edge.npts);
	}
	fprintf(fpam,"\n@4\n");
	for (i=0;i<net->ne;i++) {
		edge = net->edgeList[i];
		for (k=0;k<edge.npts;k++) {
			j = edge.pt[k];
//			fprintf(fpam,"%6.1f %6.1f %6.1f\n",net->point[j].x,net->point[j].y,net->point[j].z);
			fprintf(fpam,"%6.1f %6.1f %6.1f\n",
				net->point[j].x - origin_shift[0],
				net->point[j].y - origin_shift[1],
				net->point[j].z - origin_shift[2]);
		}
	}
	fprintf(fpam,"\n@5\n");
	for (i=0;i<net->ne;i++) {
		edge = net->edgeList[i];
		for (k=0;k<edge.npts;k++) {
			j = edge.pt[k];
			fprintf(fpam,"%6.2f\n",net->point[j].d);
		}
	}
	fclose(fpam);
	printf("Completed WriteAmiraFile\n");
	return 0;
}

//-----------------------------------------------------------------------------------------------------
// Read Amira SpatialGraph file
//-----------------------------------------------------------------------------------------------------
int ReadAmiraFile(char *amFile, NETWORK *net)
{
	int i, j, k, kp, npts;
	int np_used, ne_used;
	EDGE edge;
	char line[STR_LEN];

	fprintf(fpout,"ReadAmiraFile: %s\n",amFile);
	FILE *fpam = fopen(amFile,"r");

	npts = 0;
	kp = 0;
	k = 0;
	while (k < 3) {
		fgets(line, STR_LEN, fpam);		// reads until newline character
		printf("%s\n",line);
		if (strncmp(line,"define VERTEX",13) == 0) {
			sscanf(line+13,"%d",&net->nv);
			k++;
		}
		if (strncmp(line,"define EDGE",11) == 0) {
			sscanf(line+11,"%d",&net->ne);
			k++;
		}
		if (strncmp(line,"define POINT",12) == 0) {
			sscanf(line+12,"%d",&net->np);
			k++;
		}
	}

	net->vertex = (VERTEX *)malloc(net->nv*sizeof(VERTEX));
	net->edgeList = (EDGE *)malloc(net->ne*sizeof(EDGE));
	net->point = (POINT *)malloc(net->np*sizeof(POINT));
	printf("Allocated arrays: %d %d %d\n",net->np,net->nv,net->ne);

	// Initialize
	for (i=0; i<net->ne; i++) {
		net->edgeList[i].used = false;
	}
	for (i=0; i<net->np; i++) {
		net->point[i].used = false;
	}
	printf("Initialised\n");

	while (1) {
		if (fgets(line, STR_LEN, fpam) == NULL) {
			printf("Finished reading SpatialGraph file\n\n");
			fclose(fpam);
			break;
		}
		if (line[0] == '@') {
			sscanf(line+1,"%d",&k);
			if (k == 1) {
				for (i=0;i<net->nv;i++) {
					if (fgets(line, STR_LEN, fpam) == NULL) {
						printf("ERROR reading section @1\n");
						return 1;
					}
					sscanf(line,"%f %f %f\n",&(net->vertex[i].point.x),&(net->vertex[i].point.y),&(net->vertex[i].point.z));
					kp = i;
					net->vertex[i].point.d = 0;
					net->point[kp] = net->vertex[i].point;
				}
				kp++;
				printf("Got vertices\n");
			} else if (k == 2) {
				for (i=0;i<net->ne;i++) {
					if (fgets(line, STR_LEN, fpam) == NULL) {
						printf("ERROR reading section @2\n");
						return 1;
					}
					sscanf(line,"%d %d",&net->edgeList[i].vert[0],&net->edgeList[i].vert[1]);
					net->edgeList[i].used = true;
				}
				printf("Got edge vertex indices\n");
			} else if (k == 3) {
				for (i=0;i<net->ne;i++) {
					if (fgets(line, STR_LEN, fpam) == NULL) {
						printf("ERROR reading section @3\n");
						return 1;
					}
					sscanf(line,"%d",&net->edgeList[i].npts);
					if (net->edgeList[i].npts < 1) {
						printf("ReadAmiraFile: i: %d npts: %d\n",i,net->edgeList[i].npts);
						return 1;
					}
					net->edgeList[i].npts_used = net->edgeList[i].npts;
					net->edgeList[i].pt = (int *)malloc(net->edgeList[i].npts*sizeof(int));
					net->edgeList[i].pt_used = (int *)malloc(net->edgeList[i].npts*sizeof(int));
					npts += net->edgeList[i].npts;
					net->edgeList[i].pt[0] = net->edgeList[i].vert[0];
					net->edgeList[i].pt[net->edgeList[i].npts-1] = net->edgeList[i].vert[1];
				}
				printf("Got edge npts, total: %d\n",npts);
			} else if (k == 4) {
				for (i=0;i<net->ne;i++) {
					edge = net->edgeList[i];
					float len = 0;
					for (k=0;k<edge.npts;k++) {
						if (fgets(line, STR_LEN, fpam) == NULL) {
							printf("ERROR reading section @4\n");
							return 1;
						}
						if (k > 0 && k<edge.npts-1) {
							sscanf(line,"%f %f %f",&net->point[kp].x,&net->point[kp].y,&net->point[kp].z);
							net->edgeList[i].pt[k] = kp;
							net->edgeList[i].pt_used[k] = kp;
							kp++;
						}
						if (k > 0) {
							len = len + dist(net,net->edgeList[i].pt[k-1],net->edgeList[i].pt[k]);
						}
					}
					net->edgeList[i].length_vox = len;
				}
			} else if (k == 5) {
				for (i=0;i<net->ne;i++) {
					edge = net->edgeList[i];
					float dave = 0;
					for (k=0;k<edge.npts;k++) {
						if (fgets(line, STR_LEN, fpam) == NULL) {
							printf("ERROR reading section @5\n");
							return 1;
						}
						j = edge.pt[k];
						sscanf(line,"%f",&net->point[j].d);
						if (j == 13) {
							printf("j=13: d: %f\n",net->point[j].d);
						}
						if (net->point[j].d == 0) {
							printf("Error: ReadAmiraFile: zero diameter: i: %d npts: %d k: %d j: %d\n",i,edge.npts,k,j);
							return 1;
						}
						if (j < net->nv) {		// because the first nv points are vertices
							net->vertex[j].point.d = net->point[j].d;
						}
						dave += net->point[j].d;
						net->edgeList[i].segavediam = dave/edge.npts;
					}
				}
				printf("Got point thicknesses\n");
			}
		}
	}
	// Flag used points
	for (i=0; i<net->ne; i++) {
		edge = net->edgeList[i];
		for (k=0; k<edge.npts; k++) {
			j = edge.pt[k];
			net->point[j].used = true;
		}
	}
	fclose(fpam);
	np_used = 0;
	for (j=0; j<net->np; j++) {
		if (net->point[j].used) np_used++;
	}
	printf("Points: np: %d np_used: %d\n",net->np,np_used);
	ne_used = 0;
	for (j=0; j<net->ne; j++) {
		if (net->edgeList[j].used) ne_used++;
	}
	printf("Edges: ne: %d ne_used: %d\n",net->ne,ne_used);
	return 0;
}

//-----------------------------------------------------------------------------------------------------
// Check for vertex index iv in ivlist.  If it exists, return the index. 
// Otherwise add it to the list, increment nv, return the index.
//-----------------------------------------------------------------------------------------------------
int ivlistAdd(int iv, int *ivlist, int *nv)
{
	if (*nv == 0) {
		ivlist[0] = iv;
		*nv = 1;
		return 0;
	}
	for (int i=0; i<*nv; i++) {
		if (iv == ivlist[i]) {
			return i;
		}
	}
	ivlist[*nv] = iv;
	(*nv)++;
	return *nv-1;
}

//-----------------------------------------------------------------------------------------------------
// Locate all edges with at least one vertex within the sphere of radius R, centre (x,y,z)
// If cube_flag = 1, use a cube.
//-----------------------------------------------------------------------------------------------------
int CreateZoomNet(NETWORK *net0, NETWORK *net1, float x, float y, float z, float R, int cube_flag)
{
	int i, j, ie, iv, kp, kv, ne, nv;
	float xv, yv, zv, r;
	bool in;
	VERTEX v;
	EDGE *elist;
	int *ivlist;

	ne = 0;
	for (i=0; i<net0->ne; i++) {
		in = false;
		for (j=0; j<2; j++) {
			iv = net0->edgeList[i].vert[j];
			v = net0->vertex[iv];
			xv = v.point.x;
			yv = v.point.y;
			zv = v.point.z;
			if (cube_flag == 0) {	// sphere
				r = sqrt((x-xv)*(x-xv) + (y-yv)*(y-yv) + (z-zv)*(z-zv));
				if (r <= R) {
					in = true;
				}
			} else {				// cube
				if (abs(x-xv) <= R && abs(y-yv) <= R && abs(z-zv) <= R) {
					in = true;
				}
			}
		}
		if (in) ne++;
	}
	printf("ne: %d\n",ne);
	// Create a list of edges, and a list of vertices
	// The index number of a vertex in the list replaces vert[] in the elist entry.
	elist = (EDGE *)malloc(ne*sizeof(EDGE));
	ivlist = (int *)malloc(2*ne*sizeof(int));
	nv = 0;
	ne = 0;
	for (i=0; i<net0->ne; i++) {
		in = false;
		for (j=0; j<2; j++) {
			iv = net0->edgeList[i].vert[j];
			v = net0->vertex[iv];
			xv = v.point.x;
			yv = v.point.y;
			zv = v.point.z;
			if (cube_flag == 0) {	// sphere
				r = sqrt((x-xv)*(x-xv) + (y-yv)*(y-yv) + (z-zv)*(z-zv));
				if (r <= R) {
					in = true;
				}
			} else {				// cube
				if (abs(x-xv) <= R && abs(y-yv) <= R && abs(z-zv) <= R) {
					in = true;
				}
			}
		}
		if (in) {
			elist[ne] = net0->edgeList[i];
			for (j=0; j<2; j++) {
				iv = net0->edgeList[i].vert[j];
				kv = ivlistAdd(iv,ivlist,&nv);
				elist[ne].vert[j] = kv;
			}
			ne++;
		}
	}
	printf("ne: %d  nv: %d\n",ne,nv);
	// Now from this list of edges we need to create the network net1.
	// Note that the vertex indices in ivlist[] are for the net0 vertex list.
	net1->vertex = (VERTEX *)malloc(nv*sizeof(VERTEX));
	net1->edgeList = (EDGE *)malloc(ne*sizeof(EDGE));
	net1->point = (POINT *)malloc(net0->np*sizeof(POINT));
	// Set up net1 vertices
	for (iv=0; iv<nv; iv++) {
		net1->vertex[iv] = net0->vertex[ivlist[iv]];
	}
	// Set up net1 edges
	for (i=0; i<ne; i++) {
		net1->edgeList[i] = elist[i];
	}
	// Copy net0 points to net1, initially set all points unused
	for (i=0; i<net0->np; i++) {
		net1->point[i] = net0->point[i];
		net1->point[i].used = false;
	}
	// Now flag those points that are used.
	for (ie=0; ie<ne; ie++) {
		for (kp=0; kp<elist[ie].npts; kp++) {
			i = elist[ie].pt[kp];
			net1->point[i].used = true;
		}
	}

	net1->ne = ne;
	net1->np = net0->np;
	net1->nv = nv;
	return 0;
}

//-----------------------------------------------------------------------------------------------------
// This code assumes that the selection of a region of interest (a cube) is carried out on the 3D image.
// This means that the cube centre (xc,yc,zc) and the width (diameter) are all specified in voxel
// coordinates.  The values are converted to um by multiplying by voxelsize in um.  This is necessary
// in order to specify the corresponding region in the Amira network (in which the distance unit is um).
// To enable comparison of the zoomed network file with the cropped 3D image, either the network must
// be scaled to use voxelsize as the distance unit (in which case direct comparison is possible in Amira),
// or the network file coordinates can be left in units of um (in which case the voxelsize must be specified
// when the image file is imported into Amira).  The second option has been adopted.
//-----------------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int err;
	char *input_amfile;
	char drive[32], dir[128],filename[256], ext[32];
	char errfilename[256], output_amfile[256], result_file[256];
	char output_basename[256];
	int cmgui_flag, cube_flag, origin_shift_flag;
	int vox_xc, vox_yc, vox_zc;
	float xc, yc, zc, R, diameter, voxelsize, origin_shift[3];
	NETWORK *NP0, *NP1;

	if (argc != 11) {
		printf("Usage: zoom input_amfile output_amfile xc yc zc diameter cube_flag voxelsize cmgui_flag origin_shift_flag\n");
		fperr = fopen("zoom_error.log","w");
		fprintf(fperr,"Usage: zoom input_amfile output_amfile xc yc zc diameter cube_flag voxelsize cmgui_flag origin_shift_flag\n");
		fprintf(fperr,"Submitted command line: argc: %d\n",argc);
		for (int i=0; i<argc; i++) {
			fprintf(fperr,"argv: %d: %s\n",i,argv[i]);
		}
		fclose(fperr);
		return 1;	// Wrong command line
	}

	input_amfile = argv[1];
	strcpy(output_amfile,argv[2]);
	sscanf(argv[3],"%d",&vox_xc);
	sscanf(argv[4],"%d",&vox_yc);
	sscanf(argv[5],"%d",&vox_zc);
	sscanf(argv[6],"%f",&diameter);
	sscanf(argv[7],"%d",&cube_flag);
	sscanf(argv[8],"%f",&voxelsize);
	sscanf(argv[9],"%d",&cmgui_flag);
	sscanf(argv[10],"%d",&origin_shift_flag);
	_splitpath(output_amfile,drive,dir,filename,ext);
	strcpy(output_basename,drive);
	strcat(output_basename,dir);
	strcat(output_basename,filename);
	sprintf(errfilename,"%s_zoom.log",output_basename);
	sprintf(result_file,"%s_zoom.out",output_basename);
	fperr = fopen(errfilename,"w");

//	fprintf(fperr,"drive: %s dir: %s filename: %s ext: %s\n",drive,dir,filename,ext);
//	fprintf(fperr,"Basename: %s\n",output_basename);

	xc = voxelsize*vox_xc;
	yc = voxelsize*vox_yc;
	zc = voxelsize*vox_zc;
	R = voxelsize*diameter/2;
	fpout = fopen(result_file,"w");	
	NP0 = (NETWORK *)malloc(sizeof(NETWORK));
	err = ReadAmiraFile(input_amfile,NP0);
	if (err != 0) return 2;
	NP1 = (NETWORK *)malloc(sizeof(NETWORK));
	err = CreateZoomNet(NP0,NP1,xc,yc,zc,R,cube_flag);
	if (err != 0) return 3;

	if (origin_shift_flag == 1) {
		origin_shift[0] = xc - R;
		origin_shift[1] = yc - R;
		origin_shift[2] = zc - R;
//		ShiftOrigin(NP1,origin_shift);  NO!
	} else {
		origin_shift[0] = 0;
		origin_shift[1] = 0;
		origin_shift[2] = 0;
	}
	fprintf(fperr,"origin_shift: %f %f %f\n",origin_shift[0],origin_shift[1],origin_shift[2]);

	err = WriteAmiraFile(output_amfile,input_amfile,NP1,origin_shift);
	if (err != 0) return 4;
	if (cmgui_flag == 1) {
		err = WriteCmguiData(output_basename,NP1,origin_shift);
		if (err != 0) return 5;
	}
	return 0;
}
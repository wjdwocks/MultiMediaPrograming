#define TRANSPARENCY_VALUE (0.6f)

enum QUAD
{
	TOP_LEFT, TOP_RIGHT, BOTTOM_RIGHT, BOTTOM_LEFT, NUM_QUADRANTS
};

const int DEFAULT_QUADRANT = BOTTOM_RIGHT;

const float X_EDGE_BUFFER = 0.025f;
const float Y_EDGE_BUFFER = 0.05f;

const float SECONDARY_WIDTH = 0.4f;
const float SECONDARY_HEIGHT = 0.4f;

const int PRIMARY_STREAM = 0;
const int SECONDARY_STREAM = 1;


const int ZORDER_CLOSE = 0;
const int ZORDER_FAR = 1;

const float MOVEVAL = 0.05f;

const int SWAP_NOTHING = 0;
const int SWAP_POSITION = 1;
const int SWAP_ALPHA = 2;

const int UPDATE_TIMER = 2000;
const int POSITION_TIMEOUT = 100;
const int ALPHA_TIMEOUT = 50;

const float NUM_ANIMATION_STEPS = 10.0f;

typedef struct
{
	FLOAT				xPos, yPos;
	FLOAT				xSize, ySize;
	FLOAT				fAlpha;
	BOOL				bFlipped, bMirrored;
	VMR9NormalizedRect	rect;
}STRM_PARAM;

typedef struct
{
	float	xPos, yPos;
	float	xSize, ySize;
}QUADRANT;

HRESULT UpdatePinAlpha(int nStreamID);
HRESULT UpdatePinPos(int nStreamID);

void InitStreamParams(void);
void PositionStream(int nStream, int nQuadrant);
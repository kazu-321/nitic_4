#include "circle.xpm"
#include "cross.xpm"
#include "none.xpm"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define NONE 0
#define CIRCLE 1
#define CROSS 2

#define BOARD_SIZE 3
#define BASE_CELL_SIZE 100
#define BUF_SIZE 4096

#define TARGET_HOST "api.islaytouch.com"
#define TARGET_PORT 80
#define TARGET_PATH "/ttt.php"

#define PROXY_HOST "po.cc.ibaraki-ct.ac.jp"
#define PROXY_PORT 3128

#define STUDENT_KEY "2023149"

unsigned char board[BOARD_SIZE][BOARD_SIZE] = {NONE};
unsigned char currentColor                  = CIRCLE;
char          buf[BUF_SIZE];

Display *display;
Window   window;
Colormap colormap;
GC       gc;
XEvent   event;
Pixmap   sourceTile[3];
Pixmap   scaledTile[3];

static bool useProxy = false;
static int  windowWidth  = BOARD_SIZE * BASE_CELL_SIZE;
static int  windowHeight = BOARD_SIZE * BASE_CELL_SIZE;
static int  boardOriginX  = 0;
static int  boardOriginY  = 0;
static int  cellSize      = BASE_CELL_SIZE;
static int  boardPixel    = BOARD_SIZE * BASE_CELL_SIZE;
static int  lastCellSize  = -1;
static bool gameOver      = false;
static unsigned char gameResult = NONE;
static long long resultStartMs = 0;
static const long long RESULT_DISPLAY_MS = 5000;
static int  lastResultSecond = -1;
static bool hasWinLine    = false;
static int  winLineKind    = -1;
static int  winLineIndex   = -1;
static XFontStruct *resultFont = NULL;
static int  lastResultFontSize = -1;

void createWindow (int left, int top, char *title);
void onEvent (void);
bool placeStone (int x, int y);
void drawBoard (void);
void resetGame (void);
bool checkWin (unsigned char color);
bool isBoardFull (void);
void announceResult (unsigned char winner);
void sendMoveToServer (void);
void urlEncode (const char *src, char *dest);
bool parseBoardResponse (const char *body);
void boardToString (char *dest, size_t size);
void updateBoardGeometry (void);
void rebuildScaledTiles (void);
Pixmap scalePixmapNearest (Pixmap src, int width, int height);
void freeScaledTiles (void);
bool pointToCell (int x, int y, int *gridX, int *gridY);
void drawCircleMark (int cellX, int cellY);
void drawCrossMark (int cellX, int cellY);
bool setWinningLine (unsigned char color);
void drawWinningLine (void);
void drawResultOverlay (unsigned char winner);
void drawCenteredText (const char *text, int y, int pixelSize);
long long monotonicMs (void);
int remainingResultSeconds (void);
void clearWinningLine (void);
XFontStruct *ensureResultFont (int pixelSize);

int main (int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp (argv[i], "-p") == 0) {
            useProxy = true;
        }
    }

    createWindow (BASE_CELL_SIZE * BOARD_SIZE, BASE_CELL_SIZE * BOARD_SIZE, "Tic-Tac-Toe");
    drawBoard ();

    while (1) {
        onEvent ();
        if (gameOver && monotonicMs () - resultStartMs >= RESULT_DISPLAY_MS) {
            resetGame ();
        } else if (gameOver) {
            int remaining = remainingResultSeconds ();
            if (remaining != lastResultSecond) {
                lastResultSecond = remaining;
                drawBoard ();
            }
        }
        usleep (1000);
    }

    XDestroyWindow (display, window);
    XCloseDisplay (display);
    return 0;
}

void createWindow (int left, int top, char *title) {
    XSetWindowAttributes attributes;
    XSizeHints           hints = {0};

    attributes.backing_store = WhenMapped;
    hints.flags              = PPosition | PSize;
    hints.x                  = left;
    hints.y                  = top;
    hints.width              = BASE_CELL_SIZE * BOARD_SIZE;
    hints.height             = BASE_CELL_SIZE * BOARD_SIZE;

    display = XOpenDisplay (NULL);
    if (display == NULL) {
        fprintf (stderr, "Cannot open X display\n");
        exit (1);
    }

    windowWidth  = hints.width;
    windowHeight = hints.height;

    window   = XCreateSimpleWindow (display, RootWindow (display, DefaultScreen (display)), left, top, hints.width, hints.height, 1, BlackPixel (display, DefaultScreen (display)), WhitePixel (display, DefaultScreen (display)));
    colormap = DefaultColormap (display, DefaultScreen (display));

    XChangeWindowAttributes (display, window, CWBackingStore, &attributes);
    XSelectInput (display, window, ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask);
    XStoreName (display, window, title);
    XSetNormalHints (display, window, &hints);
    XMapWindow (display, window);

    gc = XCreateGC (display, DefaultRootWindow (display), 0, 0);
    XpmCreatePixmapFromData (display, window, none, &sourceTile[NONE], NULL, NULL);
    XpmCreatePixmapFromData (display, window, circle, &sourceTile[CIRCLE], NULL, NULL);
    XpmCreatePixmapFromData (display, window, cross, &sourceTile[CROSS], NULL, NULL);
    updateBoardGeometry ();
    rebuildScaledTiles ();
}

void onEvent (void) {
    int gridX, gridY;

    while (XPending (display) > 0) {
        XNextEvent (display, &event);

        switch (event.type) {
            case Expose:
                drawBoard ();
                break;
            case ConfigureNotify:
                windowWidth  = event.xconfigure.width;
                windowHeight = event.xconfigure.height;
                updateBoardGeometry ();
                drawBoard ();
                break;
            case ButtonRelease:
                if (gameOver) {
                    break;
                }
                if (!pointToCell (event.xbutton.x, event.xbutton.y, &gridX, &gridY)) {
                    break;
                }
                if (placeStone (gridX, gridY)) {
                    if (checkWin (CIRCLE)) {
                        setWinningLine (CIRCLE);
                        announceResult (CIRCLE);
                        break;
                    }
                    if (isBoardFull ()) {
                        announceResult (NONE);
                        break;
                    }
                    sendMoveToServer ();
                }
                break;
        }
    }
}

bool placeStone (int x, int y) {
    if (x > -1 && x < BOARD_SIZE && y > -1 && y < BOARD_SIZE && board[y][x] == NONE) {
        board[y][x] = CIRCLE;
        drawBoard ();
        return true;
    }
    return false;
}

void drawBoard (void) {
    int x, y;

    updateBoardGeometry ();

    XSetForeground (display, gc, WhitePixel (display, DefaultScreen (display)));
    XFillRectangle (display, window, gc, 0, 0, (unsigned int)windowWidth, (unsigned int)windowHeight);

    if (boardPixel <= 0) {
        XFlush (display);
        return;
    }

    for (y = 0; y < BOARD_SIZE; y++) {
        for (x = 0; x < BOARD_SIZE; x++) {
            if (board[y][x] == CIRCLE) {
                drawCircleMark (x, y);
            } else if (board[y][x] == CROSS) {
                drawCrossMark (x, y);
            } else if (scaledTile[NONE] != None) {
                XCopyArea (display, scaledTile[NONE], window, gc, 0, 0, (unsigned int)cellSize, (unsigned int)cellSize, boardOriginX + x * cellSize, boardOriginY + y * cellSize);
            }
        }
    }

    if (hasWinLine) {
        drawWinningLine ();
    }

    if (gameOver) {
        drawResultOverlay (gameResult);
    }

    XFlush (display);
}

void resetGame (void) {
    int x, y;

    for (y = 0; y < BOARD_SIZE; y++) {
        for (x = 0; x < BOARD_SIZE; x++) {
            board[y][x] = NONE;
        }
    }

    currentColor = CIRCLE;
    gameOver     = false;
    gameResult   = NONE;
    clearWinningLine ();
    hasWinLine   = false;
    lastResultSecond = -1;
    drawBoard ();
}

void updateBoardGeometry (void) {
    int boardSide = windowWidth < windowHeight ? windowWidth : windowHeight;

    if (boardSide < BOARD_SIZE) {
        cellSize   = 1;
        boardPixel = BOARD_SIZE;
    } else {
        cellSize   = boardSide / BOARD_SIZE;
        if (cellSize < 1) cellSize = 1;
        boardPixel = cellSize * BOARD_SIZE;
    }

    boardOriginX = (windowWidth - boardPixel) / 2;
    boardOriginY = (windowHeight - boardPixel) / 2;

    if (cellSize != lastCellSize) {
        lastCellSize = cellSize;
        rebuildScaledTiles ();
    }
}

void freeScaledTiles (void) {
    int i;

    for (i = 0; i < 3; i++) {
        if (scaledTile[i] != None) {
            XFreePixmap (display, scaledTile[i]);
            scaledTile[i] = None;
        }
    }
}

void clearWinningLine (void) {
    hasWinLine  = false;
    winLineKind = -1;
    winLineIndex = -1;
}

Pixmap scalePixmapNearest (Pixmap src, int width, int height) {
    Window        root;
    int           x, y;
    unsigned int   srcWidth, srcHeight, borderWidth, depth;
    XImage       *srcImage;
    XImage       *dstImage;
    Pixmap        dst;

    if (width <= 0 || height <= 0) {
        return None;
    }

    XGetGeometry (display, src, &root, &x, &y, &srcWidth, &srcHeight, &borderWidth, &depth);
    srcImage = XGetImage (display, src, 0, 0, srcWidth, srcHeight, AllPlanes, ZPixmap);
    if (srcImage == NULL) {
        return None;
    }

    dst = XCreatePixmap (display, window, (unsigned int)width, (unsigned int)height, depth);
    dstImage = XCreateImage (display, DefaultVisual (display, DefaultScreen (display)), depth, ZPixmap, 0, NULL, (unsigned int)width, (unsigned int)height, 32, 0);
    if (dst == None || dstImage == NULL) {
        if (srcImage != NULL) XDestroyImage (srcImage);
        if (dstImage != NULL) XDestroyImage (dstImage);
        if (dst != None) XFreePixmap (display, dst);
        return None;
    }

    dstImage->data = calloc ((size_t)dstImage->bytes_per_line, (size_t)height);
    if (dstImage->data == NULL) {
        XDestroyImage (srcImage);
        dstImage->data = NULL;
        XDestroyImage (dstImage);
        XFreePixmap (display, dst);
        return None;
    }

    for (y = 0; y < height; y++) {
        int srcY = (int)((long long)y * (long long)srcHeight / height);
        if (srcY >= (int)srcHeight) srcY = (int)srcHeight - 1;
        for (x = 0; x < width; x++) {
            int srcX = (int)((long long)x * (long long)srcWidth / width);
            if (srcX >= (int)srcWidth) srcX = (int)srcWidth - 1;
            XPutPixel (dstImage, x, y, XGetPixel (srcImage, srcX, srcY));
        }
    }

    XPutImage (display, dst, gc, dstImage, 0, 0, 0, 0, (unsigned int)width, (unsigned int)height);
    XDestroyImage (srcImage);
    free (dstImage->data);
    dstImage->data = NULL;
    XDestroyImage (dstImage);
    return dst;
}

void rebuildScaledTiles (void) {
    int i;

    freeScaledTiles ();
    for (i = 0; i < 3; i++) {
        scaledTile[i] = scalePixmapNearest (sourceTile[i], cellSize, cellSize);
    }
}

bool pointToCell (int x, int y, int *gridX, int *gridY) {
    int localX;
    int localY;

    updateBoardGeometry ();

    localX = x - boardOriginX;
    localY = y - boardOriginY;
    if (localX < 0 || localY < 0 || localX >= boardPixel || localY >= boardPixel) {
        return false;
    }

    *gridX = localX / cellSize;
    *gridY = localY / cellSize;
    return (*gridX >= 0 && *gridX < BOARD_SIZE && *gridY >= 0 && *gridY < BOARD_SIZE);
}

void drawCircleMark (int cellX, int cellY) {
    int px = boardOriginX + cellX * cellSize;
    int py = boardOriginY + cellY * cellSize;
    int pad = cellSize / 10;
    int size = cellSize - pad * 2;

    if (size < 2) return;
    if (scaledTile[CIRCLE] != None) {
        XCopyArea (display, scaledTile[CIRCLE], window, gc, 0, 0, (unsigned int)cellSize, (unsigned int)cellSize, px, py);
    } else {
        XDrawArc (display, window, gc, px + pad, py + pad, (unsigned int)size, (unsigned int)size, 0, 360 * 64);
    }
}

void drawCrossMark (int cellX, int cellY) {
    int px = boardOriginX + cellX * cellSize;
    int py = boardOriginY + cellY * cellSize;
    int pad = cellSize / 10;
    int x1 = px + pad;
    int y1 = py + pad;
    int x2 = px + cellSize - pad - 1;
    int y2 = py + cellSize - pad - 1;

    if (x2 <= x1 || y2 <= y1) return;
    if (scaledTile[CROSS] != None) {
        XCopyArea (display, scaledTile[CROSS], window, gc, 0, 0, (unsigned int)cellSize, (unsigned int)cellSize, px, py);
    } else {
        XDrawLine (display, window, gc, x1, y1, x2, y2);
        XDrawLine (display, window, gc, x1, y2, x2, y1);
    }
}

bool setWinningLine (unsigned char color) {
    int i;

    clearWinningLine ();

    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i][0] == color && board[i][1] == color && board[i][2] == color) {
            hasWinLine  = true;
            winLineKind = 0;
            winLineIndex = i;
            return true;
        }
        if (board[0][i] == color && board[1][i] == color && board[2][i] == color) {
            hasWinLine  = true;
            winLineKind = 1;
            winLineIndex = i;
            return true;
        }
    }

    if (board[0][0] == color && board[1][1] == color && board[2][2] == color) {
        hasWinLine  = true;
        winLineKind = 2;
        winLineIndex = 0;
        return true;
    }
    if (board[0][2] == color && board[1][1] == color && board[2][0] == color) {
        hasWinLine  = true;
        winLineKind = 3;
        winLineIndex = 0;
        return true;
    }

    return false;
}

void drawWinningLine (void) {
    XColor color;
    int    x1, y1, x2, y2;

    color.red   = 0xE000;
    color.green = 0x2000;
    color.blue  = 0x2000;
    color.flags = DoRed | DoGreen | DoBlue;
    if (!XAllocColor (display, colormap, &color)) {
        XSetForeground (display, gc, BlackPixel (display, DefaultScreen (display)));
    } else {
        XSetForeground (display, gc, color.pixel);
    }

    if (!hasWinLine) return;

    switch (winLineKind) {
        case 0:
            x1 = boardOriginX;
            y1 = boardOriginY + winLineIndex * cellSize + cellSize / 2;
            x2 = boardOriginX + boardPixel - 1;
            y2 = y1;
            break;
        case 1:
            x1 = boardOriginX + winLineIndex * cellSize + cellSize / 2;
            y1 = boardOriginY;
            x2 = x1;
            y2 = boardOriginY + boardPixel - 1;
            break;
        case 2:
            x1 = boardOriginX;
            y1 = boardOriginY;
            x2 = boardOriginX + boardPixel - 1;
            y2 = boardOriginY + boardPixel - 1;
            break;
        case 3:
            x1 = boardOriginX + boardPixel - 1;
            y1 = boardOriginY;
            x2 = boardOriginX;
            y2 = boardOriginY + boardPixel - 1;
            break;
        default:
            return;
    }

    XSetLineAttributes (display, gc, 8, LineSolid, CapRound, JoinRound);
    XDrawLine (display, window, gc, x1, y1, x2, y2);
    XSetLineAttributes (display, gc, 1, LineSolid, CapButt, JoinMiter);
}

void drawCenteredText (const char *text, int y, int pixelSize) {
    XFontStruct *font;
    int          width;
    int          x;

    font = ensureResultFont (pixelSize);
    if (font == NULL) {
        return;
    }

    width = XTextWidth (font, text, (int)strlen (text));
    x     = (windowWidth - width) / 2;

    XSetFont (display, gc, font->fid);
    XDrawString (display, window, gc, x, y, text, (int)strlen (text));
}

void drawResultOverlay (unsigned char winner) {
    int centerY;
    int titleY;
    int subtitleY;
    int titlePixel;
    int subtitlePixel;
    int remaining;
    char restartText[32];
    char titleText[32];

    XSetForeground (display, gc, BlackPixel (display, DefaultScreen (display)));
    XSetLineAttributes (display, gc, 3, LineSolid, CapButt, JoinMiter);
    XDrawRectangle (display, window, gc, 6, 6, (unsigned int)(windowWidth - 13), (unsigned int)(windowHeight - 13));

    centerY  = windowHeight / 2;
    titlePixel = windowHeight / 18;
    if (titlePixel < 14) titlePixel = 14;
    if (titlePixel > 28) titlePixel = 28;
    subtitlePixel = windowHeight / 30;
    if (subtitlePixel < 10) subtitlePixel = 10;
    if (subtitlePixel > 16) subtitlePixel = 16;

    titleY    = centerY - titlePixel;
    subtitleY = centerY + titlePixel / 2;

    if (winner == CIRCLE) {
        strcpy (titleText, "YOU WIN");
    } else if (winner == CROSS) {
        strcpy (titleText, "SERVER WINS");
    } else {
        strcpy (titleText, "DRAW");
    }
    drawCenteredText (titleText, titleY, titlePixel);
    remaining = remainingResultSeconds ();
    if (remaining < 1) remaining = 1;
    snprintf (restartText, sizeof (restartText), "Restart %d", remaining);
    drawCenteredText (restartText, subtitleY, subtitlePixel);
}

long long monotonicMs (void) {
    struct timespec ts;

    clock_gettime (CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

int remainingResultSeconds (void) {
    long long elapsed = monotonicMs () - resultStartMs;
    long long remainMs = RESULT_DISPLAY_MS - elapsed;

    if (remainMs <= 0) return 0;
    return (int)((remainMs + 999) / 1000);
}

XFontStruct *ensureResultFont (int pixelSize) {
    char fontName[128];

    if (pixelSize == lastResultFontSize && resultFont != NULL) {
        return resultFont;
    }

    if (resultFont != NULL) {
        XFreeFont (display, resultFont);
        resultFont = NULL;
    }

    snprintf (fontName, sizeof (fontName), "-misc-fixed-bold-r-normal--%d-*-*-*-c-*-iso10646-1", pixelSize);
    resultFont = XLoadQueryFont (display, fontName);
    if (resultFont == NULL) {
        resultFont = XLoadQueryFont (display, "fixed");
    }
    lastResultFontSize = pixelSize;
    return resultFont;
}

bool checkWin (unsigned char color) {
    int i;

    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i][0] == color && board[i][1] == color && board[i][2] == color) return true;
        if (board[0][i] == color && board[1][i] == color && board[2][i] == color) return true;
    }

    if (board[0][0] == color && board[1][1] == color && board[2][2] == color) return true;
    if (board[0][2] == color && board[1][1] == color && board[2][0] == color) return true;

    return false;
}

bool isBoardFull (void) {
    int x, y;

    for (y = 0; y < BOARD_SIZE; y++) {
        for (x = 0; x < BOARD_SIZE; x++) {
            if (board[y][x] == NONE) return false;
        }
    }

    return true;
}

void announceResult (unsigned char winner) {
    gameOver   = true;
    gameResult = winner;
    resultStartMs = monotonicMs ();
    drawBoard ();

    if (winner == CIRCLE) {
        puts (">>> YOU WIN! <<<");
    } else if (winner == CROSS) {
        puts (">>> SERVER WINS! <<<");
    } else {
        puts (">>> DRAW! <<<");
    }

    puts ("Restart 5...");
    fflush (stdout);
}

void boardToString (char *dest, size_t size) {
    size_t pos = 0;
    int    x, y;

    if (size == 0) return;

    for (y = 0; y < BOARD_SIZE; y++) {
        for (x = 0; x < BOARD_SIZE; x++) {
            if (pos + 1 >= size) {
                dest[size - 1] = '\0';
                return;
            }
            dest[pos++] = (char)('0' + board[y][x]);
        }
    }

    dest[pos] = '\0';
}

void urlEncode (const char *src, char *dest) {
    while (*src) {
        if (*src == ' ') {
            strcpy (dest, "%20");
            dest += 3;
        } else if (*src == '\n') {
            /* Ignore newline characters. */
        } else {
            *dest++ = *src;
        }
        src++;
    }
    *dest = '\0';
}

bool parseBoardResponse (const char *body) {
    const char *cursor = strstr (body, "\"board\"");
    int         values[BOARD_SIZE * BOARD_SIZE];
    int         count = 0;

    if (cursor == NULL) {
        cursor = body;
    }

    cursor = strchr (cursor, '[');
    if (cursor == NULL) {
        return false;
    }

    while (*cursor != '\0' && count < BOARD_SIZE * BOARD_SIZE) {
        if (*cursor == '-' || isdigit ((unsigned char)*cursor)) {
            char *end   = NULL;
            long  value = strtol (cursor, &end, 10);
            if (end == cursor) {
                cursor++;
                continue;
            }
            values[count++] = (int)value;
            cursor          = end;
        } else {
            cursor++;
        }
    }

    if (count != BOARD_SIZE * BOARD_SIZE) {
        return false;
    }

    count = 0;
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            board[y][x] = (unsigned char)values[count++];
        }
    }

    return true;
}

void sendMoveToServer (void) {
    struct sockaddr_in serverAddr;
    struct hostent    *host;

    int   sock;
    char  request[2048];
    char  response[BUF_SIZE];
    char  body[BUF_SIZE];
    char  boardString[16];
    char  encodedBoard[64] = {0};
    char  encodedKey[64]   = {0};
    char *httpBody;
    int   totalRead = 0;
    int   n;

    const char *connectHost = useProxy ? PROXY_HOST : TARGET_HOST;
    int         connectPort = useProxy ? PROXY_PORT : TARGET_PORT;

    puts ("Waiting for server...");

    boardToString (boardString, sizeof (boardString));
    urlEncode (boardString, encodedBoard);
    urlEncode (STUDENT_KEY, encodedKey);

    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror ("Socket creation failed");
        return;
    }

    host = gethostbyname (connectHost);
    if (host == NULL) {
        perror ("Failed to resolve hostname");
        close (sock);
        return;
    }

    memset (&serverAddr, 0, sizeof (serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port   = htons (connectPort);
    memcpy (&serverAddr.sin_addr, host->h_addr, host->h_length);

    if (connect (sock, (struct sockaddr *)&serverAddr, sizeof (serverAddr)) < 0) {
        perror ("Connection failed");
        close (sock);
        return;
    }

    if (useProxy) {
        snprintf (
            request, sizeof (request),
            "GET http://%s%s?board=%s&key=%s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n\r\n",
            TARGET_HOST, TARGET_PATH, encodedBoard, encodedKey, TARGET_HOST);
    } else {
        snprintf (
            request, sizeof (request),
            "GET %s?board=%s&key=%s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n\r\n",
            TARGET_PATH, encodedBoard, encodedKey, TARGET_HOST);
    }

    if (write (sock, request, strlen (request)) < 0) {
        perror ("Write failed");
        close (sock);
        return;
    }

    memset (response, 0, sizeof (response));
    while ((n = read (sock, response + totalRead, (int)sizeof (response) - totalRead - 1)) > 0) {
        totalRead += n;
        if (totalRead >= (int)sizeof (response) - 1) break;
    }
    close (sock);

    response[totalRead] = '\0';
    httpBody            = strstr (response, "\r\n\r\n");
    if (httpBody == NULL) {
        puts ("Error: Invalid HTTP response from server.");
        return;
    }

    httpBody += 4;
    strncpy (body, httpBody, sizeof (body) - 1);
    body[sizeof (body) - 1] = '\0';

    if (!parseBoardResponse (body)) {
        puts ("Error: Invalid HTTP response from server.");
        return;
    }

    if (checkWin (CROSS)) {
        setWinningLine (CROSS);
        announceResult (CROSS);
        return;
    }

    if (checkWin (CIRCLE)) {
        setWinningLine (CIRCLE);
        announceResult (CIRCLE);
        return;
    }

    if (isBoardFull ()) {
        announceResult (NONE);
        return;
    }

    drawBoard ();
}

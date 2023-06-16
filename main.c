#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
#define CLEAR "cls"
#else // In any other OS
#define CLEAR "clear"
#endif

#define MAX_INDEX 4 // Max index of song list is 9999
#define MAX_BUFFER 100

typedef char *string;

typedef struct
{
	string name;
} Song;

struct node
{
	struct node *prev;
	Song data;
	struct node *next;
};

typedef struct node Node;

typedef struct PLAYLIST
{
	Node *head;
	Node *tail;
	int size;
} List;

typedef struct CURSOR
{
	Node *node;
	int index;
} Cursor;

struct Player
{
	ma_sound sound;
	ma_engine engine;
} player;

static string SongList[] = {
	"Adele - Easy On Me",
	"Ed Sheeran - Shivers",
	"The Kid Laroi, Justin Bieber - STAY",
	"Glass Animals - Heat Waves",
	"Ed Sheeran - Bad Habits",
	"Justin Bieber - Ghost",
	"Post Malone, Swae Lee - Sunflower",
	"Dua Lipa, DaBaby - Levitating",
	"Adele - Oh My God",
	"Kane Brown, H.E.R. - Blessed & Free",
	"NEIKED, Mae Muller, POLO G - Better Days",
	"Maroon 5, Megan Thee Stallion - Beautiful Mistakes",
	"The Weeknd - Blinding Lights",
	"Tones and I - Dance Monkey",
	"Taylor Swift - Wildest Dreams (Taylor's Version)",
	"OneRepublic - Sunshine",
	"AJR - Bang!",
	"Billie Eilish - bad guy",
	"Kane Brown, blackbear - Memory",
	"Skylar Grey, POLO G, Mozzy, Eminem - Last One Standing",
	"Justin Bieber, Daniel Caesar, Giveon - Peaches",
	"Powfu, beabadoobee - death bed (coffee for your head)",
	"24kgoldn, Iann Dior - Mood",
	"Bruno Mars, Anderson .Paak, Silk Sonic - Leave The Door Open",
	"Dua Lipa - Don't Start Now",
	"Keith Urban, P!nk - One Too Many",
	"G-Eazy, Marc E. Bassy - Faithful",
	"Kane Brown, Swae Lee, Khalid - Be Like That",
	"BTS - Dynamite",
	"Ed Sheeran - Overpass Graffiti",
	"Jonas Brothers - Who's In Your Head",
	"Shawn Mendes - It'll Be Okay",
	"Alesso, Katy Perry - When I'm Gone",
	"Sam Feldt, Rita Ora - Follow Me",
	"24kgoldn - More Than Friends",
	"Sadie Jean - WYD Now",
	"Ruel - GROWING UP IS",
	"Cheat Codes, Lee Brice, Lindsay Ell - How Do You Love",
	"Kygo, X Ambassadors - Undeniable",
	"JESSIA - But I Don'T",
	"Ed_Sheeran - I Don't Care feat. Justin Bieber",
	"Connor Price - Straight A's",
	"Ed Sheeran - Beautiful People ft Khalid",
};

#define SONG_LIST_SIZE (sizeof(SongList) / sizeof(SongList[0]))
#define SONGS_PER_PAGE 5
#define TOTAL_PAGES (SONG_LIST_SIZE / SONGS_PER_PAGE)
#define NOT_FULL_PAGE (SONG_LIST_SIZE % SONGS_PER_PAGE)
#define MAX_PAGES (TOTAL_PAGES + (NOT_FULL_PAGE > 0 ? 0 : -1))

void clearStdin()
{
	while (getchar() != '\n')
		;
}

void clearScreen() { system(CLEAR); }

char *trim(char *str)
{
	char *end;

	if (*str == 0)
		return str;

	// Trim leading space
	while (*str == ' ' || *str == '\t' || *str == '\n')
		str++;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while (end > str && (*end == ' ' || *end == '\t' || *end == '\n'))
		end--;

	// Write new null terminator character
	end[1] = '\0';

	return str;
}

int getValidInt(int min, int max, const char *message)
{
	int input;
	do
	{
		printf("%s", message);
		if (scanf("%d", &input) != 1)
		{
			clearStdin();
			printf("Invalid input. Please try again.\n");
			continue;
		}
		if (input < min || input > max)
			printf("Invalid input. Valid input is between %d and %d. Please try again.\n", min, max);
	} while (input < min || input > max);
	clearStdin();
	return input;
}

float getValidFloat(float min, float max, const char *message)
{
	float input;
	do
	{
		printf("%s", message);
		scanf("%f", &input);
		if (input < min || input > max)
			printf("Invalid input. Valid input is between %f and %f. Please try again.\n", min, max);
	} while (input < min || input > max);
	clearStdin();
	return input;
}

char getValidChar(const char *validChars, const char *message)
{
	char input;
	do
	{
		printf("%s", message);
		input = toupper(fgetc(stdin));
		clearStdin();
		if (strchr(validChars, input) == NULL)
			printf("Invalid input. Please try again.\n");
	} while (strchr(validChars, input) == NULL);
	return input;
}

void getValidString(char *string, int min, int max, const char *message)
{
	do
	{
		printf("%s", message);
		fgets(string, max, stdin);
		strncpy(string, trim(string), max);
		if (strlen(string) < min)
			printf("Invalid input. Please try again.\n");
	} while (strlen(string) < min);
}

Node *newNode()
{
	Node *node = (Node *)malloc(sizeof(Node));
	node->prev = NULL;
	node->next = NULL;
	return node;
}

List *newList()
{
	List *list = (List *)malloc(sizeof(List));
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	return list;
}

Cursor *newCursor()
{
	Cursor *cursor = (Cursor *)malloc(sizeof(Cursor));
	cursor->node = NULL;
	cursor->index = 0;
	return cursor;
}

int BinarySearch(string arr[], string x, int n)
{
	int l = 0;
	int r = n - 1;
	while (l <= r)
	{
		int m = l + (r - l) / 2;
		int res = strcmp(x, arr[m]);
		if (res == 0)
			return m + 1; // return index of song
		if (res > 0)
			l = m + 1;
		else
			r = m - 1;
	}
	return -1;
}

bool isEmpty(List *list)
{
	if (list->head == NULL)
	{
		printf("There is no song in the playlist.\n");
		return true;
	}
	else
		return false;
}

bool isSelected(Cursor *cursor)
{
	if (cursor->node == NULL)
	{
		printf("No Song is selected.\n");
		return false;
	}
	else
		return true;
}

void printPlayList(List list, Cursor cursor)
{
	Node *node = list.head;
	printf("+---------+---------------------------------------------------------------+\n");
	printf("|  Index  |                             Title                             |\n");
	printf("+---------+---------------------------------------------------------------+\n");
	int index = 1;
	char buffer[MAX_INDEX];
	while (node != NULL)
	{
		snprintf(buffer, MAX_INDEX, "%d", index);
		printf("| %7s | %61s |\n", index == cursor.index ? " * " : buffer, node->data.name);
		node = node->next;
		index++;
	}
	printf("+---------+---------------------------------------------------------------+\n");
}

void printSongList(int min, int max)
{
	printf("+---------+---------------------------------------------------------------+\n");
	printf("|  Index  |                             Title                             |\n");
	printf("+---------+---------------------------------------------------------------+\n");
	for (int i = min; i < max; i++)
	{
		printf("| %7d | %61s |\n", i + 1, SongList[i]);
	}
	printf("+---------+---------------------------------------------------------------+\n");
}

void printSongPage(int page)
{
	int min = page * SONGS_PER_PAGE;
	int max = min + SONGS_PER_PAGE;
	if (max > SONG_LIST_SIZE)
		max = SONG_LIST_SIZE;
	printSongList(min, max);
}

void addSong(List *list, Node *node)
{
	if (list->head == NULL)
	{
		list->head = node;
		list->tail = node;
	}
	else
	{
		list->tail->next = node;
		node->prev = list->tail;
		list->tail = node;
	}
	list->size++;
}

bool selectSong(List *list)
{
	int index;

	switch (getValidInt(1, 2, "1. Select from the song list\n2. Search from the song list\n"))
	{
	case 1:
		int page = 0;
		char input;
		do
		{
			clearScreen();
			printSongPage(page);
			input = getValidChar("QEPN", "N(ext page), P(revious page), E(nter) Song, Q(uit): ");
			if (input == 'N' && page < MAX_PAGES)
				page++;

			if (input == 'P' && page > 0)
				page--;

			if (input == 'Q')
				return false;

		} while (input != 'E');
		index = getValidInt(1, SONG_LIST_SIZE, "Enter the index of the song: ");
		break;

	case 2:
		char buffer[MAX_BUFFER];
		getValidString(buffer, 1, MAX_BUFFER, "Enter the name of the song: ");
		index = BinarySearch(SongList, buffer, SONG_LIST_SIZE);
		if (index == -1)
		{
			printf("Song not found.\n");
			return false;
		}
		break;

	default:
		break;
	}

	Node *node = newNode();
	node->data.name = (string)malloc(sizeof(char) * strlen(SongList[index - 1]));
	strcpy(node->data.name, SongList[index - 1]);
	addSong(list, node);
	return true;
}

void stop()
{
	if (ma_sound_is_playing(&player.sound))
	{
		ma_sound_stop(&player.sound);
		ma_engine_uninit(&player.engine);
	}
}

void play(const char *path)
{
	char buffer[strlen(path) + strlen("./music/") + strlen(".mp3") + 1];
	strcpy(buffer, "./music/");
	strcat(buffer, path);
	strcat(buffer, ".mp3");

	stop();

	ma_result result;

	result = ma_engine_init(NULL, &player.engine);
	if (result != MA_SUCCESS)
	{
		printf("Failed to initialize audio engine.");
		exit(1);
	}

	result = ma_sound_init_from_file(&player.engine, buffer, 0, NULL, NULL, &player.sound);
	if (result != MA_SUCCESS)
	{
		printf("Failed to load sound file.");
		exit(1);
	}
	ma_sound_set_looping(&player.sound, MA_TRUE);
	ma_sound_start(&player.sound);
}

void next(Cursor *cursor)
{
	if (!isSelected(cursor))
		return;

	if (cursor->node->next == NULL)
	{
		printf("This is the last song.\n");
		return;
	}

	cursor->node = cursor->node->next;
	cursor->index++;
	clearScreen();
	printf("Playing %s\n", cursor->node->data.name);
	play(cursor->node->data.name);
}

void prev(Cursor *cursor)
{
	if (!isSelected(cursor))
		return;

	if (cursor->node->prev == NULL)
	{
		printf("This is the first song.\n");
		return;
	}
	cursor->node = cursor->node->prev;
	cursor->index--;
	clearScreen();
	printf("Playing %s\n", cursor->node->data.name);
	play(cursor->node->data.name);
}

void playSong(List *list, Cursor *cursor)
{
	if (isEmpty(list))
		return;

	printPlayList(*list, *cursor);
	int num = getValidInt(1, list->size, "Enter the index of the song: ");
	if (cursor->index <= 0)
	{
		cursor->node = list->head;
		cursor->index = 1;
		for (int i = 1; i < num; i++)
			next(cursor);
	}
	else
	{
		if (num > cursor->index)
		{
			int diff = num - cursor->index;
			for (int i = 0; i < diff; i++)
				next(cursor);
		}
		else if (num < cursor->index)
		{
			int diff = cursor->index - num;
			for (int i = 0; i < diff; i++)
				prev(cursor);
		}
	}
	clearScreen();
	printf("Playing %s\n", cursor->node->data.name);
	play(cursor->node->data.name);
	printPlayList(*list, *cursor);
}

void delete(List *list, Cursor *cursor)
{
	if (!isSelected(cursor))
		return;

	Node *node = NULL;
	if (cursor->node->next == NULL && cursor->node->prev != NULL)
	{
		cursor->node = cursor->node->prev;
		node = cursor->node->next;
		cursor->node->next = NULL;
		list->tail = cursor->node;
		printf("Playing %s\n", cursor->node->data.name);
		play(cursor->node->data.name);
		if (cursor->index > 1)
			cursor->index--;
	}
	else if (cursor->node->next != NULL)
	{
		cursor->node = cursor->node->next;
		node = cursor->node->prev;
		if (node->prev != NULL)
		{
			cursor->node->prev = node->prev;
			node->prev->next = cursor->node;
		}
		else
		{
			cursor->node->prev = NULL;
			list->head = cursor->node;
		}
		printf("Playing %s\n", cursor->node->data.name);
		play(cursor->node->data.name);
	}
	else
	{
		list->head = NULL;
		list->tail = NULL;
		node = cursor->node;
		cursor->node = NULL;
		cursor->index = 0;
	}
	free(node);
	list->size--;
	isEmpty(list) ? stop() : printPlayList(*list, *cursor);
}

void deleteSong(List *list, Cursor *cursor)
{
	printPlayList(*list, *cursor);
	int num = getValidInt(1, list->size, "Enter the index of the song: ");
	if (num == cursor->index)
		delete (list, cursor);
	else
	{
		Node *node = list->head;
		for (int i = 1; i < num; i++)
			node = node->next;

		if (node->prev != NULL && node->next != NULL)
		{
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}

		if (node->prev != NULL && node->next == NULL)
		{
			list->tail = node->prev;
			node->prev->next = NULL;
		}

		if (node->prev == NULL && node->next != NULL)
		{
			list->head = node->next;
			node->next->prev = NULL;
		}

		list->size--;
		if (cursor->index > 1 && cursor->index > num)
			cursor->index--;

		node->next = NULL;
		node->prev = NULL;
		free(node);
	}
}

int MainMenu()
{
	printf("Welcome to Music Player 1.0\n");
	printf("1. Select song from the Song List\n");
	printf("2. Play the selected song\n");
	printf("3. Next song\n");
	printf("4. Previous song\n");
	printf("5. Display the current Playlist\n");
	printf("6. Delete the current song\n");
	printf("7. Delete Song from the PlayList\n");
	printf("8. Exit\n");
	return getValidInt(1, 8, "Enter your choice: ");
}

void sort()
{
	string curr;
	int i, j;
	// Insertion Sort
	for (i = 0; i < SONG_LIST_SIZE; i++)
	{
		curr = SongList[i];
		for (j = i; j > 0; j--)
		{
			if (strcmp(SongList[j - 1], curr) > 0)
				SongList[j] = SongList[j - 1];
			else
				break;
		}
		SongList[j] = curr;
	}
}

int main()
{
	sort();
	List *list = newList();
	Cursor *cursor = newCursor();
	do
	{
		clearScreen();
		switch (MainMenu())
		{
		case 1:
			!selectSong(list) ?: printf("Song added successfully.\n");
			break;

		case 2:
			isEmpty(list) ?: playSong(list, cursor);
			break;

		case 3:
			isEmpty(list) ?: next(cursor);
			break;

		case 4:
			isEmpty(list) ?: prev(cursor);
			break;

		case 5:
			isEmpty(list) ?: printPlayList(*list, *cursor);
			break;

		case 6:
			isEmpty(list) ?: delete (list, cursor);
			break;

		case 7:
			isEmpty(list) ?: deleteSong(list, cursor);
			break;

		case 8:
			exit(0);
			break;
		}
		if (getValidChar("YN", "Do you want to continue? (Y/N): ") == 'N')
			break;
	} while (1);
	return 0;
}
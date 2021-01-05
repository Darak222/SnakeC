#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>
#define ROZMIAR 255
typedef struct segment segment;
int apple_x, apple_y, score;
bool victory = false;
int width = 11;
int height = 11;

struct segment {
  char symbol[3];
  segment* nastepny;
  int x;
  int y;
};

typedef struct {
  int dx;
  int dy;
  segment *glowa;
} waz;

typedef struct sortList { //strukura pomocniczna do sortowania
  char user[ROZMIAR];
  int score;
  struct sortList *next;
} sortList;



void init_window(waz *snek) //funkcja inicjalizująca mapę
{
  printw("WYNIK: %d\n", score); 
  for(int i = 0; i < width+2; i++)
  {
    printw(" -");
  }
  printw("\n");
  for(int i = 0; i < height; i++)
  {
    printw(" |");
    for(int j = 0; j < width; j++){
      printw("  ");
    }
    printw(" |\n");
  }
  for(int i = 0; i < width+2; i++)
  {
    printw(" -");
  }
}

void my_split(const char *source, char *first, char *second, const char delim) //funkcja pomocniczna do odczytu danych z pliku z wynikami
{
  while((*source != delim)){
    *first++ = *source++;
  }
  *first = '\0';

  *source++;

  while((*source != delim)){
    *second++ = *source++;
  }
  *second = '\0';
}

void spawn_random_apple(waz *Snek) //Losowo generuje współrzędne jabłka
{
  segment *curr_segment;
  bool can_spawn;
  int random_x;
  int random_y;
  do {
    can_spawn = true;
    curr_segment = Snek->glowa;
    random_x = (rand() % height) + 2;
    random_y = (rand() % width) + 1;
    while(curr_segment != NULL){ //Sprawdzenie czy współrzędne nie są takie same jak współrzędne głowy bądź segmentu
      if(curr_segment->x == random_x && curr_segment->y == random_y){
        can_spawn = false;
        break;
      }
      curr_segment = curr_segment->nastepny;
    }
  } while (!can_spawn);
  apple_x = random_x;
  apple_y = random_y;
  
}

bool check_self_crash(waz *snek, segment *tail){ //Sprawdza czy wąż zderzył się sam ze sobą
  if(snek->glowa->x == tail->x && snek->glowa->y == tail->y){
    return true;
  }
  if(tail->nastepny == NULL){
    return false;
  }
  return check_self_crash(snek, tail->nastepny);
}

bool check_collision(waz *snek){ //Sprawdza zderzenie z granicą mapy
  bool collided = false; //Sprawdzane są specyficzne współrzędne (+2 i +1) ze względu na znaki z których zbudowana jest mapa (- oraz |)
  if(snek->glowa->x == 1 || snek->glowa->x == height + 2 || snek->glowa->y == 0 || snek->glowa->y == width + 1){
    collided = true;
  } //Sprawdznie kolizji z samym sobą
  if(snek->glowa->nastepny != NULL && check_self_crash(snek, snek->glowa->nastepny)){
    collided = true;
  }
  return collided;
}

void draw_segment(segment *tail, char tail_mark[]){ //Dorysowywanie ogona węża
  mvaddstr(tail->x, tail->y*2, tail_mark); //y*2 aby poruszanie się wyglądało lepiej
  if(tail->nastepny != NULL){
    draw_segment(tail->nastepny, tail->symbol);
  }
  strcpy(tail->symbol, tail_mark);
}

void draw(waz *snek, char tail_mark[]) //Generacja mapy, węża i jabłka
{
  clear();
  init_window(snek); //Generacja mapy
  mvaddstr(apple_x, apple_y*2, " o"); //Generacja jabłka
  mvaddstr(snek->glowa->x, snek->glowa->y*2, snek->glowa->symbol); //Generacja głowy węża z odpowiednią głową
  if(snek->glowa->nastepny != NULL){
    draw_segment(snek->glowa->nastepny, tail_mark); //Rysowanie ogona jeśli istnieje
  }
  refresh();
}

void add_segment(waz *snek){ //Zwiększa długość węża bo zjedzeniu jabłka
  segment *curr_segment = snek->glowa;
  while(curr_segment->nastepny != NULL){
    curr_segment = curr_segment->nastepny;
  }
  segment *new_segment = malloc(sizeof(segment)); //Alokacja pamięci dla nowego segmentu ogona
  new_segment->x = curr_segment->x; //Współrzędne dla nowego segmentu
  new_segment->y = curr_segment->y;
  new_segment->nastepny = NULL;
  curr_segment->nastepny = new_segment;
}

void move_next_segment(segment *tail, int x, int y){ //Ruch ogona węża
  if(tail->nastepny != NULL)
  {
    move_next_segment(tail->nastepny, tail->x, tail->y);
  }

  tail->x = x;
  tail->y = y;
}

void move_snek(waz *snek){ //Ruch węża
  bool eat_apple = false;
  if(snek->glowa->y + snek->dy == apple_y && snek->glowa->x + snek->dx == apple_x){ //Wąż zjadł jabłko
    score++;
    eat_apple = true;
    add_segment(snek);    
  }
  if(snek->glowa->nastepny != NULL){
    move_next_segment(snek->glowa->nastepny, snek->glowa->x, snek->glowa->y); //Ruch ogona jeśli istnieje
  }
  snek->glowa->y = snek->glowa->y + snek->dy; //Przemieszczenie głowy
  snek->glowa->x = snek->glowa->x + snek->dx;
  if(score == (width*height)-1){ //Jeśli gracz zdobył maksymalną ilość punktów na dany rozmiar mapy tzn. Wąż nie ma już miejsca do poruszania się
    victory = true;
  }
  else if(eat_apple){ //Jeśli wąż zjadł jabłko to wygeneruj nowe
    spawn_random_apple(snek);
  }

}

void wait_x_seconds(int seconds){ //Pomocniczna funkcja odliczająca sekundy
    time_t start_wait = time(0);
    time_t current_wait = time(0);
    do{
      current_wait = time(0);
    } while (current_wait - start_wait <= seconds);
}

void show_scores(){ //Wypisyanie wyników graczy z pliku
  int max_char = ROZMIAR;
  char score_board[max_char];
  char strA[max_char];
  char strB[max_char];
  FILE *fp;
  fp = fopen("SnekScores.txt", "r");
  if (fp == NULL){
    printw("Nie udalo sie otworzyc pliku z wynikami\n");
  }
  else{
    printw("Lista graczy oraz ich wyniki:\n");
    while(fgets(score_board, sizeof(score_board), fp)){
      my_split(score_board, strA, strB, ';');
      printw("Gracz: %s, Wynik: %s\n", strA, strB);
    }
  }
  fclose(fp);   
}

void get_sorted_scores(sortList *start){ //Sortuje wyniki algorytmem sortowania przez wybieranie
  struct sortList *node = NULL, *tmp = NULL, *max_node = NULL;
  int max_val, tmp_val;
  char max_user[ROZMIAR], tmp_user[ROZMIAR];
  node = start;

  while(node != NULL){

    tmp = node;
    max_node = node;
    max_val = node->score;

    strcpy(max_user, node->user);
    while(tmp != NULL){
      if(tmp->score > max_val){
        max_node = tmp;
        max_val = tmp->score;
        strcpy(max_user, tmp->user);
      }
      tmp = tmp->next;

    }

    tmp_val = max_val;
    strcpy(tmp_user, max_user);

    max_node->score = node->score;
    strcpy(max_node->user, node->user);

    node->score = tmp_val;
    strcpy(node->user, tmp_user);
    node = node->next;
  } 

}

void sort_scores(){ //Sortuje wyniki i posortowane wyniki wpisuje do pliku z wynikami (max 10 wyników w pliku)
  sortList *start = NULL;
  sortList *to_sort, *curr;
  int convert;
  char sort_scores[ROZMIAR];
  char strA[ROZMIAR];
  char strB[ROZMIAR];
  FILE *reader;
  reader = fopen("SnekScores.txt", "r");
  if(reader == NULL){
    printw("Nie udalo sie otworzyc pliku z wynikami\n");
    exit;
  }
  while(fgets(sort_scores, sizeof(sort_scores), reader)){ //Odczyt danych z pliku i wprowadzenie ich do linked listy sortList
    my_split(sort_scores, strA, strB, ';');
    to_sort = (struct sortList *)malloc(sizeof(sortList));
    strcpy(to_sort->user ,strA);
    convert = atoi(strB);
    to_sort->score = convert;
    to_sort->next = NULL;
    if(start == NULL){
      start = to_sort;
      curr = to_sort;
    }
    else{
      curr->next = to_sort;
      curr = to_sort;
    }
  }
  fclose(reader);
  get_sorted_scores(start); //Sortowanie
  FILE *writer;
  writer = fopen("SnekScores.txt", "w");
  if(writer == NULL){
    printw("Nie udalo sie otworzyc pliku z wynikami\n");
    exit;
  }
  curr = start;
  for(int i = 0; i < 10; i++){ //Zapis 10 najlepszych wyników graczy do bazy
    if(curr == NULL){
      break;
    }
    fputs(curr->user, writer);
    fputs(";", writer);
    fprintf(writer, "%d", curr->score);
    fputs(";\n", writer);
    curr = curr->next;
  }

  fclose(writer);




}



int main(void) 
{
  char username[50];
  int seconds = 3;
  wait_x_seconds(seconds); //Odliczanie do startu
  bool first_time = true;

  FILE* create_save = fopen("SnekScores.txt", "a");
  fclose(create_save);

  WINDOW * mainwin;
  bool again = true;

  do{
    victory = false;
    // Initializacja ncurses
    if ( (mainwin = initscr()) == NULL ) {
      fprintf(stderr, "Error initialising ncurses.\n");
      exit(EXIT_FAILURE);
    }
    if(first_time){ //Jeśli gracz rozgrywa grę po raz pierwszy to proszony jest o podanie nicku, jeśli gra ponownie to nie musi podawać nicku
      printw("Wprowadz swoj nick\n");
      getstr(username);
      first_time = false;
    }
    
    score = 0;
    sort_scores(); //Sortowanie istniejących wyników w pliku

    raw();
    keypad(stdscr, true); //Umożliwia działanie strzałek podczas sterowania
    noecho();         // nie pokazuj wpisywanych danych
    cbreak();
    timeout(500);     // czekaj 500ms na klawisz



    bool quit = false;

    srand(time(NULL));



    segment *head = malloc(sizeof(struct segment)); //alokacja pamięci dla węża
    waz *snek = malloc(sizeof(waz));

    snek->dx = 0; //inicjalizacja węża
    snek->dy = 1;

    head->nastepny = NULL; //inicjalizacja głowy
    head->x = height/2 + 2; //Pozycja startowa
    head->y = width/2 + 1;
    strcpy(head->symbol, " >"); //Startowy wygląd głowy
    snek->glowa = head;

    char tail_marker[3];

    time_t time_before = time(0); //Pomocnicza zmienna do odświeżania ekranu
    clear();


    spawn_random_apple(snek); //Początek gry
    draw(snek, tail_marker);
    do {


      if(time(0) - time_before >= 1 ){ //Odświeżanie ekranu co sekundę
        move_snek(snek);
        draw(snek, tail_marker);
        time_before = time(0);
      }
    


      int c = getch();
      switch (c) //Obsługa klawiszy strzałek
      {
        case 'q':
          quit = true;
          break;
        case KEY_UP:
          if(snek->glowa->nastepny == NULL || (snek->glowa->nastepny != NULL && snek->glowa->x - snek->glowa->nastepny->x != 1)){
            snek->dx = -1;
            snek->dy = 0;
            strcpy(snek->glowa->symbol, " ^"); //Zmiana wyglądu głowy w zależności od kierunku poruszania się
            strcpy(tail_marker, " |");
          }
          break;
        case KEY_DOWN:
          if(snek->glowa->nastepny == NULL || (snek->glowa->nastepny != NULL && snek->glowa->x - snek->glowa->nastepny->x != -1)){
            snek->dx = 1;
            snek->dy = 0;
            strcpy(snek->glowa->symbol, " v");
            strcpy(tail_marker, " |");
          }
          break;
        case KEY_LEFT:
          if(snek->glowa->nastepny == NULL || (snek->glowa->nastepny != NULL && snek->glowa->y - snek->glowa->nastepny->y != 1)){
            snek->dy = -1;
            snek->dx = 0;
            strcpy(snek->glowa->symbol, " <");
            strcpy(tail_marker, " -"); //Zmiana wyglądu ogona w zależności od kierunku
          }
          break;
        case KEY_RIGHT:
          if(snek->glowa->nastepny == NULL || (snek->glowa->nastepny != NULL && snek->glowa->y - snek->glowa->nastepny->y != -1)){
            snek->dy = 1;
            snek->dx = 0;
            strcpy(snek->glowa->symbol, " >");
            strcpy(tail_marker, " -");
          }
          break;
        default:
          break;
      }
    
      if(check_collision(snek)){ //Sprawdzenie warunków końca gry
        quit = true;
      }
      if(victory){ 
        quit = true;
      }
    } while( ! quit );



    while(quit){
      clear();
      printw("--- Koniec Gry ---\n"); //Ekran końca gry
      printw("Twoj wynik to: %d\n", score);

      show_scores();

      FILE *fps;
      fps = fopen("SnekScores.txt", "a");

      printw("Czy chcesz zapisac swoj wynik? y/n\n");
      refresh();
      int w = getch();
      switch (w)
      {
        case 'y':
          if(fps == NULL){
            printw("Nie udalo sie zapisac wyniku :(");
            break;
          }
          fputs(username, fps);
          fputs(";", fps);
          fprintf(fps, "%d", score);
          fputs(";\n", fps);
          fclose(fps);
          quit = false;
          break;
        case 'n':
          fclose(fps);
          quit = false;
          break;
        default:
          break;
      }
    }

    while(!quit){
      clear();
      printw("Czy chcesz zagrac ponownie? y/n\n"); //Ewentualne ponowne rozpoczącie rozgrywki bez konieczności ponownego uruchomienia aplikacji
      refresh();
      int c = getch();
      switch (c) 
      {
        case 'n':
          quit = true;
          again = false;
          break;
        case 'y':
          clear();
          printw("Nowa gra rozpocznie sie za %d sekund...", seconds);
          refresh();
          quit = true;
          break;
        default:
          break;
      }
    } 
    if(again){
      wait_x_seconds(seconds); //Odliczanie przed ponownym rozpoczęciem
    }
    
  } while (again);

  // Czyszczenie
  nocbreak();
  echo();
  refresh();
  delwin(mainwin);
  endwin();
  sort_scores();


  return EXIT_SUCCESS;
}

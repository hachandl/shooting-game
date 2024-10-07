#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <string.h>
#include <wchar.h>



#pragma region 키보드 
#define LEFT 75
#define RIGHT 77
#define SPACE 32
#pragma endregion
#pragma region 벨런스
#define INITIAL_ENEMY_SPAWN_DELAY 750 // 초기 적 소환 간격 (밀리초)
#define INITIAL_ENEMY_MOVE_DELAY 750  // 초기 적 이동 간격 (밀리초)
#define MAX_ENEMIES 1000 //최대 적의 개수
#pragma endregion

int enemy_count = 0; // 현재 적 개수
int canAttack = 1;
int score = 0; // 점수 변수
int health = 3; // 플레이어 체력
int isInvincible = 0; // 무적 상태를 추적하는 변수 추가
int invincibleDuration = 100; // 무적 상태 유지 시간 (밀리초)
CRITICAL_SECTION cs; //멀티 스레드

#pragma region 위치 함수
void Character_Position(int x, int y)
{
    COORD character_position = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), character_position);
}
void Attack_Position(int x, int y)
{
    COORD attack_position = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), attack_position);
}
void Enemy_Position(int x, int y)
{
    COORD enemy_position = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), enemy_position);
}
#pragma endregion
#pragma region 구조체
typedef struct Character
{
    int x, y;
    const char* shape;
} Character;
typedef struct AttackParams
{
    int x;
    int y;
} AttackParams;
typedef struct Enemy
{
    int x, y;
    const char* shape;
    int active;
} Enemy;
#pragma endregion
#pragma region 맵
char Map[32][22] = {
    {'1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','1'},
    {'1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1'},
};
void Rendering_Map()
{
    EnterCriticalSection(&cs); // Enter Critical Section
    printf("\n\n");
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 22; j++)
        {
            if (Map[i][j] == '0')
            {
                printf("  ");
            }
            else if (Map[i][j] == '1')
            {
                printf("■");
            }
        }
        printf("\n");
    }
    LeaveCriticalSection(&cs); // Leave Critical Section
}
#pragma endregion

Enemy enemies[MAX_ENEMIES]; // 적 배열

//커서 삭제
void CursorView()
{
    CONSOLE_CURSOR_INFO cursorInfo = { 0, };
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}
// 점수 및 체력 표시 함수
void DisplayStatus()
{
    EnterCriticalSection(&cs); // Enter Critical Section
    COORD status_position = { 0, 0 }; // 점수와 체력을 표시할 위치 설정
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), status_position);
    printf("Score: %d    Health: ", score); // 점수 표시

    // 체력을 '♡'로 표시
    for (int i = 0; i < health; i++) {
        printf("♡ ");
    }

    LeaveCriticalSection(&cs); // Leave Critical Section
}
// 게임 오버 메시지 출력 함수
void GameOver()
{
    EnterCriticalSection(&cs);
    system("cls"); // 화면을 지움
    printf("===== GAME OVER =====\n");
    printf("Your Score: %d\n", score);
    printf("Press ENTER to exit...");
    LeaveCriticalSection(&cs);

    // Enter 키 입력 대기
    while (1)
    {
        if (_getch() == 13) // Enter 키
        {
            exit(0); // 게임 종료
        }
    }
}
// 플레이어 공격 함수
void Attack(int x, int y, wchar_t ch)
{
    EnterCriticalSection(&cs); // Enter Critical Section
    COORD pos = { x, y };
    DWORD written;
    WriteConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), &ch, 1, pos, &written);
    LeaveCriticalSection(&cs); // Leave Critical Section
}

// 적과 플레이어 충돌 체크 함수
void CheckCollisionWithEnemy(Character* player)
{
    if (!isInvincible) // 무적 상태가 아닐 때만 충돌 처리
    {
        for (int i = 0; i < enemy_count; i++)
        {
            if (enemies[i].active && enemies[i].y == player->y && abs(enemies[i].x - player->x) <= 1)
            {
                // 충돌 발생
                health--; // 플레이어 체력 감소
                if (health <= 0)
                {
                    GameOver(); // 체력이 0이면 게임 오버
                }
                DisplayStatus(); // 체력 갱신

                // 적 비활성화
                enemies[i].active = 0;
                EnterCriticalSection(&cs);
                Enemy_Position(enemies[i].x, enemies[i].y);
                printf("  "); // 적을 지움
                LeaveCriticalSection(&cs);

                // 무적 상태 부여
                isInvincible = 1;
                Sleep(invincibleDuration); // 무적 시간 동안 대기
                isInvincible = 0; // 무적 상태 해제

                break; // 하나의 적과만 충돌 처리
            }
        }
    }
}

// 적 이동 및 충돌 처리 함수
void MoveEnemies(int moveDelay, Character* player)
{
    while (1)
    {
        for (int i = 0; i < enemy_count; i++)
        {
            if (enemies[i].active)
            {
                EnterCriticalSection(&cs);
                Enemy_Position(enemies[i].x, enemies[i].y);
                printf("  "); // 공백으로 지우기
                enemies[i].y += 1;

                if (enemies[i].y >= 33) // 맵을 이탈한 적은 비활성화
                {
                    enemies[i].active = 0;
                }
                else
                {
                    Enemy_Position(enemies[i].x, enemies[i].y);
                    printf("%s", enemies[i].shape);
                }
                LeaveCriticalSection(&cs);
            }
        }
        Sleep(moveDelay);
        moveDelay -= 3;

        // 충돌 체크 (매번 적 이동 후 체크)
        CheckCollisionWithEnemy(player);
    }
}
// 적 소환 함수
void SpawnEnemy(int* spawnDelay)
{
    if (enemy_count < MAX_ENEMIES)
    {
        int random_x = rand() % 20;
        enemies[enemy_count].x = random_x * 2 + 2;
        enemies[enemy_count].y = 3;
        enemies[enemy_count].shape = "▽";
        enemies[enemy_count].active = 1;
        enemy_count++;
    }
}

#pragma region 스레드 함수
// 플레이어 공격 스레드 함수
DWORD WINAPI PlayerAttack(LPVOID lpParam)
{
    AttackParams* params = (AttackParams*)lpParam;
    for (int i = params->y - 1; i >= 3; i--)
    {
        Attack(params->x, i, L'|');
        Sleep(100);

        for (int j = 0; j < enemy_count; j++)
        {
            if (enemies[j].active && enemies[j].x == params->x && enemies[j].y == i)
            {
                enemies[j].active = 0;
                EnterCriticalSection(&cs);
                Enemy_Position(enemies[j].x, enemies[j].y);
                printf("  "); // 공백 출력으로 지우기
                LeaveCriticalSection(&cs);

                score += 100;
                DisplayStatus();

                free(params);
                return 0;
            }
        }

        Attack(params->x, i, L' ');
    }

    free(params);
    return 0;
}
// 적 소환 스레드 함수
DWORD WINAPI EnemySpawn(LPVOID lpParam)
{
    int spawnDelay = INITIAL_ENEMY_SPAWN_DELAY;
    while (1)
    {
        SpawnEnemy(&spawnDelay);
        Sleep(spawnDelay);
        spawnDelay -= 3;
    }
}
// 적 이동 스레드 함수
DWORD WINAPI EnemyMove(LPVOID lpParam)
{
    Character* player = (Character*)lpParam;
    MoveEnemies(INITIAL_ENEMY_MOVE_DELAY, player);
    return 0;
}

#pragma endregion

// 플레이어
void Player()
{
    Character character = { 2, 32, "▲" };
    char key = 0;
    int prev_x = character.x;

    while (1)
    {
        if (_kbhit())
        {
            key = _getch();

            if (key == -32)
            {
                key = _getch();
            }

            EnterCriticalSection(&cs); // 좌표 조작을 위한 동기화
            switch (key)
            {
            case LEFT:
                if (character.x > 2 && Map[character.y][character.x / 2 - 1] != '1')
                {
                    prev_x = character.x;
                    character.x -= 2;
                }
                break;

            case RIGHT:
                if (character.x < 40 && Map[character.y][character.x / 2 + 1] != '1')
                {
                    prev_x = character.x;
                    character.x += 2;
                }
                break;
            case SPACE:
                AttackParams* params = (AttackParams*)malloc(sizeof(AttackParams));
                params->x = character.x;
                params->y = character.y;
                CreateThread(NULL, 0, PlayerAttack, (LPVOID)params, 0, NULL);
                break;
            }

            // 이전 위치 지우기
            Character_Position(prev_x, character.y);
            printf("  ");

            // 새로운 위치에 플레이어 출력
            Character_Position(character.x, character.y);
            printf("%s", character.shape);
            LeaveCriticalSection(&cs); // 동기화 해제
        }

        Sleep(50);
    }
}

int main()
{
    system("mode con:cols=50 lines=35");
    InitializeCriticalSection(&cs); // 크리티컬 섹션 초기화
    CursorView(); // 커서 삭제
    Rendering_Map(); // 맵
    srand((unsigned int)time(NULL)); // 랜덤 시드 설정
    DisplayStatus(); // 점수 및 체력 초기 표시
    Character player = { 2, 32, "▲" }; // 플레이어 캐릭터 생성
    CreateThread(NULL, 0, EnemySpawn, NULL, 0, NULL); // 적 스폰 스레드 생성
    CreateThread(NULL, 0, EnemyMove, (LPVOID)&player, 0, NULL);  // 적 이동 스레드 생성

    Player(); // 플레이어 함수 실행

    DeleteCriticalSection(&cs); // 크리티컬 섹션 해제
    return 0;
}

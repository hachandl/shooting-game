#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <string.h>
#include <wchar.h>

#pragma region Ű���� 
#define LEFT 75
#define RIGHT 77
#define SPACE 32
#pragma endregion
#pragma region ������
#define MAX_ENEMY_ATTACKS_PER_SHOT 10    // �� ���� �߻��ϴ� �ִ� �� ���� ��
#define ENEMY_ATTACK_INTERVAL 1500       // �� ���� ���� (�и���)
#define ENEMY_ATTACK_SPEED 100           // �� ���� �̵� �ӵ� (�и���)
#define MAX_ENEMY_PROJECTILES 1000       // ���� �� ���ݷ� (���Ѵ�� ���� ����)
#define INITIAL_ENEMY_SPAWN_DELAY 750 // �ʱ� �� ��ȯ ���� (�и���)
#define INITIAL_ENEMY_MOVE_DELAY 750  // �ʱ� �� �̵� ���� (�и���)
#define MAX_ENEMIES 1000 //�ִ� ���� ����
#define ATTACK_COOLDOWN 500 // ���� �� ��� �ð�
#pragma endregion

int enemy_count = 0; // ���� �� ����
int canAttack = 1;
int score = 0; // ���� ����
int health = 3; // �÷��̾� ü��
int isInvincible = 0; // ���� ���¸� �����ϴ� ����
int invincibleDuration = 100; // ���� ���� ���� �ð� (�и���)
int isGameOver = 0; // ���� ���� ���¸� �����ϴ� ����
CRITICAL_SECTION cs; //��Ƽ ������

#pragma region ��ġ �Լ�
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
#pragma region ����ü
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
typedef struct EnemyAttack
{
    int x, y;      // ������ ���� ��ġ
    int active;    // ������ Ȱ��ȭ�Ǿ����� ����
} EnemyAttack;
#pragma endregion
#pragma region ��
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
                printf("��");
            }
        }
        printf("\n");
    }
    LeaveCriticalSection(&cs); // Leave Critical Section
}
#pragma endregion

Enemy enemies[MAX_ENEMIES]; // �� �迭
EnemyAttack enemy_attacks[MAX_ENEMY_PROJECTILES]; // �� ���� �迭
//Ŀ�� ����
void CursorView()
{
    CONSOLE_CURSOR_INFO cursorInfo = { 0, };
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}
// ���� �� ü�� ǥ�� �Լ�
void DisplayStatus()
{
    EnterCriticalSection(&cs); // Enter Critical Section
    COORD status_position = { 0, 0 }; // ������ ü���� ǥ���� ��ġ ����
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), status_position);
    printf("Score: %d    Health: ", score); // ���� ǥ��

    for (int i = 0; i < health; i++) {
        printf("�� ");
    }

    LeaveCriticalSection(&cs); // Leave Critical Section
}
// ���� ���� �޽��� ��� �Լ�
void GameOver()
{
    EnterCriticalSection(&cs);
    system("cls"); // ȭ���� ����
    printf("===== GAME OVER =====\n");
    printf("Your Score: %d\n", score);
    printf("Press ENTER to exit...");
    LeaveCriticalSection(&cs);

    isGameOver = 1;  // ���� ���� ���·� ����

    // Enter Ű �Է� ���
    while (1)
    {
        if (_getch() == 13) // Enter Ű
        {
            exit(0); // ���� ����
        }
    }
}
// �÷��̾� ���� �Լ�
void Attack(int x, int y, wchar_t ch)
{
    EnterCriticalSection(&cs); // Enter Critical Section
    COORD pos = { x, y };
    DWORD written;
    WriteConsoleOutputCharacter(GetStdHandle(STD_OUTPUT_HANDLE), &ch, 1, pos, &written);
    LeaveCriticalSection(&cs); // Leave Critical Section
}
// ���� �÷��̾� �浹 üũ �Լ�
void CheckCollisionWithEnemy(Character* player)
{
    if (!isInvincible) // ���� ���°� �ƴ� ���� �浹 ó��
    {
        for (int i = 0; i < enemy_count; i++)
        {
            if (enemies[i].active && enemies[i].y == player->y && abs(enemies[i].x - player->x) <= 1)
            {
                // �浹 �߻�
                health--; // �÷��̾� ü�� ����
                if (health <= 0)
                {
                    GameOver(); // ü���� 0�̸� ���� ����
                }
                DisplayStatus(); // ü�� ����

                // �� ��Ȱ��ȭ
                enemies[i].active = 0;
                EnterCriticalSection(&cs);
                Enemy_Position(enemies[i].x, enemies[i].y);
                printf("  "); // ���� ����
                LeaveCriticalSection(&cs);

                // ���� ���� �ο�
                isInvincible = 1;
                Sleep(invincibleDuration); // ���� �ð� ���� ���
                isInvincible = 0; // ���� ���� ����

                break; // �ϳ��� ������ �浹 ó��
            }
        }
    }
}
// �� �̵� �� �浹 ó�� �Լ�
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
                printf("  "); // �������� �����
                enemies[i].y += 1;

                if (enemies[i].y >= 33) // ���� ��Ż�� ���� ��Ȱ��ȭ
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

        // moveDelay�� 100ms ���Ϸ� �پ���� �ʵ��� ����
        if (moveDelay > 100) {
            moveDelay -= 3;
        }
        else {
            moveDelay = 100;
        }

        Sleep(moveDelay);

        // �浹 üũ (�Ź� �� �̵� �� üũ)
        CheckCollisionWithEnemy(player);
    }
}
// �� ��ȯ �Լ�
void SpawnEnemy(int* spawnDelay)
{
    if (enemy_count < MAX_ENEMIES)
    {
        int random_x = rand() % 20;
        enemies[enemy_count].x = random_x * 2 + 2;
        enemies[enemy_count].y = 3;
        enemies[enemy_count].shape = "��";
        enemies[enemy_count].active = 1;
        enemy_count++;
    }
}
// �� ���� �̵� �Լ�
void EnemyAttackMove()
{
    while (!isGameOver)  // ���� ���� ���°� �ƴ� ���� ���� ����
    {
        for (int i = 0; i < MAX_ENEMY_PROJECTILES; i++)
        {
            if (enemy_attacks[i].active)
            {
                EnterCriticalSection(&cs);
                Attack_Position(enemy_attacks[i].x, enemy_attacks[i].y);
                printf("  ");  // ���� ���� ��ġ �����

                enemy_attacks[i].y += 1;  // ���� �Ʒ��� �̵�

                // ���� ����ų� �� �Ʒ����� ���� ���� ����
                if (enemy_attacks[i].y >= 33 || enemy_attacks[i].y == 32)
                {
                    enemy_attacks[i].active = 0;
                }
                else
                {
                    // ���� ��ġ�� �ٸ� ������Ʈ(�� �Ǵ� �÷��̾�)�� �ִ��� Ȯ��
                    int isOnEnemy = 0;
                    for (int j = 0; j < enemy_count; j++)
                    {
                        if (enemies[j].active && enemies[j].x == enemy_attacks[i].x && enemies[j].y == enemy_attacks[i].y)
                        {
                            isOnEnemy = 1;  // �ش� ��ġ�� ���� ����
                            break;
                        }
                    }

                    // ���� ������ ������ ǥ��
                    if (!isOnEnemy)
                    {
                        Attack_Position(enemy_attacks[i].x, enemy_attacks[i].y);
                        printf("|");
                    }
                    else
                    {
                        // ���� ��ġ�� ���� ���� ���, ���� �ٽ� ���
                        Enemy_Position(enemy_attacks[i].x, enemy_attacks[i].y);
                        printf("��");
                    }
                }
                LeaveCriticalSection(&cs);
            }
        }
        Sleep(ENEMY_ATTACK_SPEED);  // �� ���� �̵� �ӵ�
    }
}
// �� ���� �߻� �Լ�
void SpawnEnemyAttack()
{
    while (1)
    {
        int shotsFired = 0;  // ���� �߻��� �� ���� ��

        while (shotsFired < MAX_ENEMY_ATTACKS_PER_SHOT) // �� ���� �ִ� 10�� �߻�
        {
            int random_enemy_index = rand() % enemy_count; // ���� �� ����

            if (enemies[random_enemy_index].active)  // Ȱ��ȭ�� ���� �߻�
            {
                for (int j = 0; j < MAX_ENEMY_PROJECTILES; j++)
                {
                    if (!enemy_attacks[j].active)  // ��Ȱ��ȭ�� ���� ���� ã��
                    {
                        enemy_attacks[j].x = enemies[random_enemy_index].x;    // ���� ��ġ���� �߻�
                        enemy_attacks[j].y = enemies[random_enemy_index].y + 1;  // ���� �ٷ� �Ʒ����� ����
                        enemy_attacks[j].active = 1;  // ���� Ȱ��ȭ

                        shotsFired++;  // �߻��� ���� �� ����
                        break;
                    }
                }
            }
        }

        Sleep(ENEMY_ATTACK_INTERVAL);  // �� ���� ����
    }
}

#pragma region ������ �Լ�
// �÷��̾� ���� ������ �Լ�
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
                printf("  "); // ���� ������� �����
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
// �� ��ȯ ������ �Լ�
WORD WINAPI EnemySpawn(LPVOID lpParam)
{
    int spawnDelay = INITIAL_ENEMY_SPAWN_DELAY;
    while (!isGameOver) // ���� ���� ���°� �ƴ� ���� �� ��ȯ
    {
        SpawnEnemy(&spawnDelay);

        // spawnDelay�� 200ms ���Ϸ� �پ���� �ʵ��� ����
        if (spawnDelay > 200) {
            spawnDelay -= 3;
        }
        else {
            spawnDelay = 200;
        }

        Sleep(spawnDelay);
    }
    return 0;
}

// �� �̵� ������ �Լ�
DWORD WINAPI EnemyMove(LPVOID lpParam)
{
    Character* player = (Character*)lpParam;
    while (!isGameOver)  // ���� ���� ���°� �ƴ� ���� �� �̵�
    {
        MoveEnemies(INITIAL_ENEMY_MOVE_DELAY, player);
    }
    return 0;
}
DWORD WINAPI ResetAttack(LPVOID lpParam)
{
    Sleep(ATTACK_COOLDOWN); // ���� �� ��� �ð�
    canAttack = 1; // �ٽ� ���� ����
    return 0;
}
#pragma endregion

// �÷��̾�
void Player()
{
    Character character = { 2, 32, "��" };
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

            EnterCriticalSection(&cs); // ��ǥ ������ ���� ����ȭ
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
                if (canAttack) // ���� ������ ���� ���� ����
                {
                    canAttack = 0; // ���� �Ұ��� ���·� ����
                    AttackParams* params = (AttackParams*)malloc(sizeof(AttackParams));
                    params->x = character.x;
                    params->y = character.y;
                    CreateThread(NULL, 0, PlayerAttack, (LPVOID)params, 0, NULL);

                    // ���� �ð��� ���� �� �ٽ� ���� �����ϵ��� ����
                    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ResetAttack, NULL, 0, NULL);
                }
                break;
            }

            // ���� ��ġ �����
            Character_Position(prev_x, character.y);
            printf("  ");

            // ���ο� ��ġ�� �÷��̾� ���
            Character_Position(character.x, character.y);
            printf("%s", character.shape);
            LeaveCriticalSection(&cs); // ����ȭ ����
        }

        Sleep(50);
    }
}

int main()
{
    system("mode con:cols=44 lines=35");
    InitializeCriticalSection(&cs); // ũ��Ƽ�� ���� �ʱ�ȭ
    CursorView(); // Ŀ�� ����
    Rendering_Map(); // ��
    srand((unsigned int)time(NULL)); // ���� �õ� ����
    DisplayStatus(); // ���� �� ü�� �ʱ� ǥ��
    Character player = { 2, 32, "��" }; // �÷��̾� ĳ���� ����
    CreateThread(NULL, 0, EnemySpawn, NULL, 0, NULL); // �� ���� ������ ����
    CreateThread(NULL, 0, EnemyMove, (LPVOID)&player, 0, NULL);  // �� �̵� ������ ����
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)EnemyAttackMove, NULL, 0, NULL);  // �� ���� �̵�
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SpawnEnemyAttack, NULL, 0, NULL);  // �� ���� �߻�

 
    Player(); // �÷��̾� �Լ� ����

    DeleteCriticalSection(&cs); // ũ��Ƽ�� ���� ����
    return 0;
}

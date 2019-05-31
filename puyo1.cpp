//課題1
//2019/04/12

#include <stdlib.h>
#include <curses.h>
#include <random>
#include <iostream>
#include <unistd.h>

//ぷよの色を表すの列挙型
//NONEが無し，RED,BLUE,..が色を表す
enum puyocolor { NONE, RED, BLUE, GREEN, YELLOW };

void DisplayPuyo(puyocolor puyo, int y, int x);

class PuyoArray
{
private:
	puyocolor *data; //盤面状態
	unsigned int data_line; //盤面の行数
	unsigned int data_column; //盤面の列数

	//メモリ解放
	void Release()
	{
		if (data == NULL) {
			return;
		}

		delete[] data;
		data = NULL;
	}

public:
	PuyoArray()
	{
		data = NULL;
		data_line = 0;
		data_column = 0;
	}

	~PuyoArray()
	{
		Release();
	}

	//盤面サイズ変更
	void ChangeSize(unsigned int line, unsigned int column)
	{
		Release();

		//新しいサイズでメモリ確保
		data = new puyocolor[line*column];

		data_line = line;
		data_column = column;
	}

	//盤面の行数を返す
	unsigned int GetLine()
	{
		return data_line;
	}

	//盤面の列数を返す
	unsigned int GetColumn()
	{
		return data_column;
	}

	//盤面の指定された位置の値を返す
	puyocolor GetValue(unsigned int y, unsigned int x)
	{
		if (y >= GetLine() || x >= GetColumn())
		{
			//引数の値が正しくない
			return NONE;
		}

		return data[y*GetColumn() + x];
	}

	//盤面の指定された位置に値を書き込む
	void SetValue(unsigned int y, unsigned int x, puyocolor value)
	{
		if (y >= GetLine() || x >= GetColumn())
		{
			//引数の値が正しくない
			return;
		}

		data[y*GetColumn() + x] = value;
	}
};

class PuyoArrayActive : public PuyoArray
{
private:
	int rotatepuyo; // ぷよの回転状態

public:
	PuyoArrayActive()
	{
		rotatepuyo = 0;
	}

	int GetRotatePuyo()
	{
		// ゲッター
		return rotatepuyo;
	}

	void SetRotatePuyo(int rotate)
	{
		// セッター
		if (0 > rotate || rotate > 3)
		{
			//引数がおかしい
			return;
		}

		rotatepuyo = rotate;
	}
};

class PuyoArrayStack : public PuyoArray
{

};

class PuyoControl
{
public:
	//ぷよぷよの色を決定
	void DecidePuyoColor(puyocolor & puyo)
	{
		std::random_device rand;
		std::mt19937 mt(rand());
		int color_num = mt()%4;

		if (color_num == 0) {
			puyo = RED;
		} else if (color_num == 1) {
			puyo = BLUE;
		} else if (color_num == 2) {
			puyo = GREEN;
		} else if (color_num == 3) {
			puyo = YELLOW;
		}
	}

	//盤面に新しいぷよ生成
	void GeneratePuyo(PuyoArrayActive &activePuyo)
	{
		puyocolor newpuyo1;
		puyocolor newpuyo2;

		DecidePuyoColor(newpuyo1);
		DecidePuyoColor(newpuyo2);

		activePuyo.SetValue(0, 5, newpuyo1);
		activePuyo.SetValue(0, 6, newpuyo2);
		activePuyo.SetRotatePuyo(0);
	}

	//ぷよの着地判定．着地判定があるとtrueを返す
	bool LandingPuyo(PuyoArrayActive &activePuyo, PuyoArrayStack &stackedPuyo)
	{
		bool landed = false;

		//これ以上積めない時はゲームオーバー
		if (stackedPuyo.GetValue(1, 5) != NONE || stackedPuyo.GetValue(1, 6) != NONE)
		{
			//上までぷよが積まれていたらゲーム終了
			endwin();
			std::cout << "GAME OVER" << std::endl;
			exit(0);
		}

		for (int y = 0; y < activePuyo.GetLine(); y++)
		{
			for (int x = 0; x < activePuyo.GetColumn(); x++)
			{
				puyocolor active = activePuyo.GetValue(y, x);
				puyocolor preActive = activePuyo.GetValue(y, x - 1);
				puyocolor nextActive = activePuyo.GetValue(y, x + 1);
				puyocolor aboveActive = activePuyo.GetValue(y - 1, x);
				puyocolor underActive = activePuyo.GetValue(y + 1, x);

				if (active != NONE)
				{
					//ぷよがある
					if (stackedPuyo.GetValue(y + 1, x) != NONE)
					{
						//一個下に着地済みのぷよがある
						landed = true;
						stackedPuyo.SetValue(y, x, active);
						activePuyo.SetValue(y, x, NONE);
						if (preActive != NONE)
						{
							//左隣にぷよがある時
							stackedPuyo.SetValue(y, x-1, preActive);
							activePuyo.SetValue(y, x-1, NONE);
						}
						else if (nextActive != NONE)
						{
							//右隣にぷよがある時
							stackedPuyo.SetValue(y, x+1, nextActive);
							activePuyo.SetValue(y, x+1, NONE);
						}
						else if (aboveActive != NONE)
						{
							//真上にぷよがある
							stackedPuyo.SetValue(y - 1, x, aboveActive);
							activePuyo.SetValue(y - 1, x, NONE);
						}
						else if (underActive != NONE)
						{
							//真下にぷよがある
							stackedPuyo.SetValue(y + 1, x, underActive);
							activePuyo.SetValue(y + 1, x, NONE);
						}
					}

					//版の一番下に到達
					if (y == activePuyo.GetLine() - 1)
					{
						landed = true;
						stackedPuyo.SetValue(y, x, active);
						activePuyo.SetValue(y, x, NONE);
					}
				}
			}
		}

		return landed;
	}

	void TearOffPuyo(PuyoArrayStack &stackedPuyo)
	{
		for (int y = 0; y < stackedPuyo.GetLine(); y++)
		{
			for (int x = 0; x < stackedPuyo.GetColumn(); x++)
			{
				puyocolor stackedColor = stackedPuyo.GetValue(y, x);

				// 着地済みぷよがあり、その下に何もない時、そのぷよを着地するまで落とす
				if (stackedColor != NONE && stackedPuyo.GetValue(y + 1, x) == NONE)
				{
					stackedPuyo.SetValue(y, x, NONE);
					while (true)
					{
						if (y == stackedPuyo.GetLine() - 1 || stackedPuyo.GetValue(y + 1, x) != NONE)
						{
							stackedPuyo.SetValue(y, x, stackedColor);
							break;
						}
						y++;
					}
				}
			}
		}
	}

	//左移動
	void MoveLeft(PuyoArrayActive &activePuyo, PuyoArrayStack &stackedPuyo)
	{
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[activePuyo.GetLine()*activePuyo.GetColumn()];

		for (int i = 0; i < activePuyo.GetLine()*activePuyo.GetColumn(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ左の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = 0; y < activePuyo.GetLine(); y++)
		{
			for (int x = 0; x < activePuyo.GetColumn(); x++)
			{
				if (activePuyo.GetValue(y, x) == NONE) {
					continue;
				}

				//端ではない、右隣に移動中、着地済みのぷよがない時に移動
				if (0 < x && activePuyo.GetValue(y, x - 1) == NONE && stackedPuyo.GetValue(y, x - 1) == NONE)
				{
					puyo_temp[y*activePuyo.GetColumn() + (x - 1)] = activePuyo.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					activePuyo.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y*activePuyo.GetColumn() + x] = activePuyo.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < activePuyo.GetLine(); y++)
		{
			for (int x = 0; x < activePuyo.GetColumn(); x++)
			{
				activePuyo.SetValue(y, x, puyo_temp[y*activePuyo.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
	}

	//右移動
	void MoveRight(PuyoArrayActive &activePuyo, PuyoArrayStack &stackedPuyo)
	{
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[activePuyo.GetLine()*activePuyo.GetColumn()];

		for (int i = 0; i < activePuyo.GetLine()*activePuyo.GetColumn(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ右の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = 0; y < activePuyo.GetLine(); y++)
		{
			for (int x = activePuyo.GetColumn() - 1; x >= 0; x--)
			{
				if (activePuyo.GetValue(y, x) == NONE) {
					continue;
				}

				//端ではない、右隣に移動中、着地済みのぷよがない時に移動
				if (x < activePuyo.GetColumn() - 1 && activePuyo.GetValue(y, x + 1) == NONE && stackedPuyo.GetValue(y, x + 1) == NONE)
				{
					puyo_temp[y*activePuyo.GetColumn() + (x + 1)] = activePuyo.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					activePuyo.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y*activePuyo.GetColumn() + x] = activePuyo.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < activePuyo.GetLine(); y++)
		{
			for (int x = 0; x < activePuyo.GetColumn(); x++)
			{
				activePuyo.SetValue(y, x, puyo_temp[y*activePuyo.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
	}

	//下移動
	void MoveDown(PuyoArray &activePuyo, PuyoArrayStack &stackedPuyo)
	{
		//一時的格納場所メモリ確保
		puyocolor *puyo_temp = new puyocolor[activePuyo.GetLine()*activePuyo.GetColumn()];

		for (int i = 0; i < activePuyo.GetLine()*activePuyo.GetColumn(); i++)
		{
			puyo_temp[i] = NONE;
		}

		//1つ下の位置にpuyoactiveからpuyo_tempへとコピー
		for (int y = activePuyo.GetLine() - 1; y >= 0; y--)
		{
			for (int x = 0; x < activePuyo.GetColumn(); x++)
			{
				if (activePuyo.GetValue(y, x) == NONE) {
					continue;
				}

				if (y < activePuyo.GetLine() - 1 && activePuyo.GetValue(y + 1, x) == NONE && stackedPuyo.GetValue(y + 1, x) == NONE)
				{
					puyo_temp[(y + 1)*activePuyo.GetColumn() + x] = activePuyo.GetValue(y, x);
					//コピー後に元位置のpuyoactiveのデータは消す
					activePuyo.SetValue(y, x, NONE);
				}
				else
				{
					puyo_temp[y*activePuyo.GetColumn() + x] = activePuyo.GetValue(y, x);
				}
			}
		}

		//puyo_tempからpuyoactiveへコピー
		for (int y = 0; y < activePuyo.GetLine(); y++)
		{
			for (int x = 0; x < activePuyo.GetColumn(); x++)
			{
				activePuyo.SetValue(y, x, puyo_temp[y*activePuyo.GetColumn() + x]);
			}
		}

		//一時的格納場所メモリ解放
		delete[] puyo_temp;
	}

	// 回転
	void Rotate(PuyoArrayActive &activePuyo, PuyoArrayStack &stackedPuyo)
	{
		// 盤面の左上から順に activepuyo を探して先のものを１、後のものを２とする
		puyocolor puyocolor1;
		puyocolor puyocolor2;
		unsigned int x1 = 0;
		unsigned int x2 = 0;
		unsigned int y1 = 0;
		unsigned int y2 = 0;

		// ぷよを探索
		bool findingpuyo1 = true;
		for (int y = 0; y < activePuyo.GetLine(); y++)
		{
			for (int x = 0; x < activePuyo.GetColumn(); x++)
			{
				if (activePuyo.GetValue(y, x) != NONE)
				{
					if (findingpuyo1 == true)
					{
						puyocolor1 = activePuyo.GetValue(y, x);
						x1 = x;
						y1 = y;
						findingpuyo1 = false;
					}
					else if (findingpuyo1 != true)
					{
						puyocolor2 = activePuyo.GetValue(y, x);
						x2 = x;
						y2 = y;
					}
				}
			}
		}

		// 開店前にぷよを消す
		activePuyo.SetValue(y1, x1, NONE);
		activePuyo.SetValue(y2, x2, NONE);

		// ぷよの回転
		switch (activePuyo.GetRotatePuyo()) {
			case 0:
				// 開店後に番外に出るときは回転しない
				if (x2 <= 0 || y2 >= activePuyo.GetLine() - 1)
				{
					activePuyo.SetValue(y1, x1, puyocolor1);
					activePuyo.SetValue(y2, x2, puyocolor2);
					break;
				}

				// 回転後に着地済みのぷよと重なるときは回転しない
				if (stackedPuyo.GetValue(y2 + 1, x2 - 1) != NONE)
				{
					activePuyo.SetValue(y1, x1, puyocolor1);
					activePuyo.SetValue(y2, x2, puyocolor2);
					break;
				}

				/* AB -> A
				         B */
				activePuyo.SetValue(y1, x1, puyocolor1);
				activePuyo.SetValue(y2 + 1, x2 - 1, puyocolor2);
				activePuyo.SetRotatePuyo(1);
				break;

			case 1:
				// 開店後に番外に出るときは回転しない
				if (x2 <= 0 || y2 <= 0)
				{
					activePuyo.SetValue(y1, x1, puyocolor1);
					activePuyo.SetValue(y2, x2, puyocolor2);
					break;
				}

				// 回転後に着地済みプヨと重なるときは回転しない
				if (stackedPuyo.GetValue(y2 - 1, x2 - 1) != NONE)
				{
					activePuyo.SetValue(y1, x1, puyocolor1);
					activePuyo.SetValue(y2, x2, puyocolor2);
					break;
				}

				/* A -> BA
				   B       */
				activePuyo.SetValue(y1, x1, puyocolor1);
				activePuyo.SetValue(y2 - 1, x2 - 1, puyocolor2);
				activePuyo.SetRotatePuyo(2);
				break;

			case 2:
				// 開店後に番外に出るときは回転しない
				if (x1 >= activePuyo.GetColumn() - 1 || y1 <= 0)
				{
					activePuyo.SetValue(y1, x1, puyocolor1);
					activePuyo.SetValue(y2, x2, puyocolor2);
					break;
				}

				// 回転後に着地済みプヨと重なるときは回転しない
				if (stackedPuyo.GetValue(y1 - 1, x1 + 1) != NONE)
				{
					activePuyo.SetValue(y1, x1, puyocolor1);
					activePuyo.SetValue(y2, x2, puyocolor2);
					break;
				}

				/*    -> A
				   AB    B */
				activePuyo.SetValue(y1 - 1, x1 + 1, puyocolor1);
				activePuyo.SetValue(y2, x2, puyocolor2);
				activePuyo.SetRotatePuyo(3);
				break;

			case 3:
				// 開店後に番外に出るときは回転しない
				if (x1 >= activePuyo.GetColumn() - 1 || y1 >= activePuyo.GetLine() - 1)
				{
					activePuyo.SetValue(y1, x1, puyocolor1);
					activePuyo.SetValue(y2, x2, puyocolor2);
					break;
				}

				// 回転した後に着地済みプヨと重なるときは回転しない
				if (stackedPuyo.GetValue(y1 + 1, x1 + 1) != NONE)
				{
					activePuyo.SetValue(y1, x1, puyocolor1);
					activePuyo.SetValue(y2, x2, puyocolor2);
					break;
				}

				/* A -> AB
				   B       */
				activePuyo.SetValue(y1 + 1, x1 + 1, puyocolor1);
				activePuyo.SetValue(y2, x2, puyocolor2);
				activePuyo.SetRotatePuyo(0);
				break;

			default:
				break;
		}
	}

	//ぷよ消滅処理を全座標で行う
	//消滅したぷよの数を返す
	int VanishPuyo(PuyoArrayStack &puyostack)
	{
		int vanishednumber = 0;
		for (int y = 0; y < puyostack.GetLine(); y++)
		{
			for (int x = 0; x < puyostack.GetColumn(); x++)
			{
				vanishednumber += VanishPuyo(puyostack, y, x);
			}
		}

		return vanishednumber;
	}

	//ぷよ消滅処理を座標(x,y)で行う
	//消滅したぷよの数を返す
	int VanishPuyo(PuyoArrayStack &puyostack, unsigned int y, unsigned int x)
	{
		//判定個所にぷよがなければ処理終了
		if (puyostack.GetValue(y, x) == NONE)
		{
			return 0;
		}


		//判定状態を表す列挙型
		//NOCHECK判定未実施，CHECKINGが判定対象，CHECKEDが判定済み
		enum checkstate{ NOCHECK, CHECKING, CHECKED };

		//判定結果格納用の配列
		enum checkstate *field_array_check;
		field_array_check = new enum checkstate[puyostack.GetLine()*puyostack.GetColumn()];

		//配列初期化
		for (int i = 0; i < puyostack.GetLine()*puyostack.GetColumn(); i++)
		{
			field_array_check[i] = NOCHECK;
		}

		//座標(x,y)を判定対象にする
		field_array_check[y*puyostack.GetColumn() + x] = CHECKING;

		//判定対象が1つもなくなるまで，判定対象の上下左右に同じ色のぷよがあるか確認し，あれば新たな判定対象にする
		bool checkagain = true;
		while (checkagain)
		{
			checkagain = false;

			for (int y = 0; y < puyostack.GetLine(); y++)
			{
				for (int x = 0; x < puyostack.GetColumn(); x++)
				{
					//(x,y)に判定対象がある場合
					if (field_array_check[y*puyostack.GetColumn() + x] == CHECKING)
					{
						//(x+1,y)の判定
						if (x < puyostack.GetColumn() - 1)
						{
							//(x+1,y)と(x,y)のぷよの色が同じで，(x+1,y)のぷよが判定未実施か確認
							if (puyostack.GetValue(y, x + 1) == puyostack.GetValue(y, x) && field_array_check[y*puyostack.GetColumn() + (x + 1)] == NOCHECK)
							{
								//(x+1,y)を判定対象にする
								field_array_check[y*puyostack.GetColumn() + (x + 1)] = CHECKING;
								checkagain = true;
							}
						}

						//(x-1,y)の判定
						if (x > 0)
						{
							if (puyostack.GetValue(y, x - 1) == puyostack.GetValue(y, x) && field_array_check[y*puyostack.GetColumn() + (x - 1)] == NOCHECK)
							{
								field_array_check[y*puyostack.GetColumn() + (x - 1)] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y+1)の判定
						if (y < puyostack.GetLine() - 1)
						{
							if (puyostack.GetValue(y + 1, x) == puyostack.GetValue(y, x) && field_array_check[(y + 1)*puyostack.GetColumn() + x] == NOCHECK)
							{
								field_array_check[(y + 1)*puyostack.GetColumn() + x] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y-1)の判定
						if (y > 0)
						{
							if (puyostack.GetValue(y - 1, x) == puyostack.GetValue(y, x) && field_array_check[(y - 1)*puyostack.GetColumn() + x] == NOCHECK)
							{
								field_array_check[(y - 1)*puyostack.GetColumn() + x] = CHECKING;
								checkagain = true;
							}
						}

						//(x,y)を判定済みにする
						field_array_check[y*puyostack.GetColumn() + x] = CHECKED;
					}
				}
			}
		}

		//判定済みの数をカウント
		int puyocount = 0;
		for (int i = 0; i < puyostack.GetLine()*puyostack.GetColumn(); i++)
		{
			if (field_array_check[i] == CHECKED)
			{
				puyocount++;
			}
		}

		//4個以上あれば，判定済み座標のぷよを消す
		int vanishednumber = 0;
		if (4 <= puyocount)
		{
			for (int y = 0; y < puyostack.GetLine(); y++)
			{
				for (int x = 0; x < puyostack.GetColumn(); x++)
				{
					if (field_array_check[y*puyostack.GetColumn() + x] == CHECKED)
					{
						puyostack.SetValue(y, x, NONE);

						vanishednumber++;
					}
				}
			}
		}

		//メモリ解放
		delete[] field_array_check;

		return vanishednumber;
	}
};

void DisplayPuyo(puyocolor puyo, int y, int x)
{
	switch (puyo)
	{
	case RED:
		attrset(COLOR_PAIR(1));
		mvaddch(y, x, 'R');
		break;
	case BLUE:
		attrset(COLOR_PAIR(2));
		mvaddch(y, x, 'B');
		break;
	case GREEN:
		attrset(COLOR_PAIR(3));
		mvaddch(y, x, 'G');
		break;
	case YELLOW:
		attrset(COLOR_PAIR(4));
		mvaddch(y, x, 'Y');
		break;
	default:
		mvaddch(y, x, '?');
		break;
	}
}

//表示
void Display(PuyoArrayActive &activePuyo, PuyoArrayStack &stackedPuyo)
{
	//落下中ぷよ表示
	for (int y = 0; y < activePuyo.GetLine(); y++)
	{
		for (int x = 0; x < activePuyo.GetColumn(); x++)
		{
			puyocolor activeColor = activePuyo.GetValue(y, x);
			puyocolor stackedColor = stackedPuyo.GetValue(y, x);
			if (activeColor == NONE && stackedColor == NONE)
			{
				attrset(0);
				mvaddch(y, x, '.');
			} else if (activeColor != NONE) {
				DisplayPuyo(activeColor, y, x);
			} else if (stackedColor != NONE) {
				DisplayPuyo(stackedColor, y, x);
			}
		}
	}

	//情報表示
	int count = 0;
	for (int y = 0; y < activePuyo.GetLine(); y++)
	{
		for (int x = 0; x < activePuyo.GetColumn(); x++)
		{
			if (activePuyo.GetValue(y, x) != NONE)
			{
				count++;
			}
		}
	}

	char msg[256];
	sprintf(msg, "Field: %d x %d, Puyo number: %03d", activePuyo.GetLine(), activePuyo.GetColumn(), count);
	mvaddstr(2, COLS - 35, msg);

	refresh();
}


//ここから実行される
int main(int argc, char **argv){
	//PuyoArray puyo;
	PuyoArrayActive activePuyo;
	PuyoArrayStack stackedPuyo;
	PuyoControl control;

	//画面の初期化
	initscr();
	//カラー属性を扱うための初期化
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);

	//キーを押しても画面に表示しない
	noecho();
	//キー入力を即座に受け付ける
	cbreak();

	curs_set(0);
	//キー入力受付方法指定
	keypad(stdscr, TRUE);

	//キー入力非ブロッキングモード
	timeout(0);

	//初期化処理
	activePuyo.ChangeSize(LINES/2, COLS/2);	//フィールドは画面サイズの縦横1/2にする
	stackedPuyo.ChangeSize(LINES/2, COLS/2);
	control.GeneratePuyo(activePuyo);	//最初のぷよ生成

	int delay = 0;
	int waitCount = 20000;

	int puyostate = 0;

	//メイン処理ループ
	while (1)
	{
		//キー入力受付
		int ch;
		ch = getch();

		//Qの入力で終了
		if (ch == 'Q')
		{
			break;
		}

		//入力キーごとの処理
		switch (ch)
		{
		case KEY_LEFT:
			control.MoveLeft(activePuyo, stackedPuyo);
			break;
		case KEY_RIGHT:
			control.MoveRight(activePuyo, stackedPuyo);
			break;
		case KEY_DOWN:
			control.MoveDown(activePuyo, stackedPuyo);
			break;
		case KEY_UP:
			control.Rotate(activePuyo, stackedPuyo);
			break;
		default:
			break;
		}


		//処理速度調整のためのif文
		if (delay%waitCount == 0){
			//ぷよ下に移動
			control.MoveDown(activePuyo, stackedPuyo);

			//ぷよ着地判定
			if (control.LandingPuyo(activePuyo, stackedPuyo))
			{
				//着地していたら消えるぷよを探して新しいぷよ生成
				control.TearOffPuyo(stackedPuyo);
				while (control.VanishPuyo(stackedPuyo) > 0) {
					Display(activePuyo, stackedPuyo);
					usleep(1000000);
					control.TearOffPuyo(stackedPuyo);
				}
				control.GeneratePuyo(activePuyo);
			}
		}
		delay++;

		//表示
		Display(activePuyo, stackedPuyo);
	}


	//画面をリセット
	endwin();

	return 0;
}

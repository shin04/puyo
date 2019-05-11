//課題1
//2019/04/12

#include <curses.h>
#include <random>

//ぷよの色を表すの列挙型
//NONEが無し，RED,BLUE,..が色を表す
enum puyocolor { NONE, RED, BLUE, GREEN, YELLOW };

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
	}
	//ぷよの着地判定．着地判定があるとtrueを返す
	bool LandingPuyo(PuyoArrayActive &activePuyo, PuyoArrayStack &stackedPuyo)
	{
		bool landed = false;

		for (int y = 0; y < activePuyo.GetLine(); y++)
		{
			for (int x = 0; x < activePuyo.GetColumn(); x++)
			{
				if (activePuyo.GetValue(y, x) != NONE)
				{
					if (stackedPuyo.GetValue(y + 1, x) != NONE)
					{
						landed = true;
						stackedPuyo.SetValue(y, x, activePuyo.GetValue(y, x));
						activePuyo.SetValue(y, x, NONE);
						if (activePuyo.GetValue(y, x-1) != NONE)
						{
							stackedPuyo.SetValue(y, x-1, activePuyo.GetValue(y, x-1));
							activePuyo.SetValue(y, x-1, NONE);
						}
						else if (activePuyo.GetValue(y, x+1) != NONE)
						{
							stackedPuyo.SetValue(y, x+1, activePuyo.GetValue(y, x+1));
							activePuyo.SetValue(y, x+1, NONE);
						}
					}

					if (y == activePuyo.GetLine() - 1)
					{
						landed = true;
						stackedPuyo.SetValue(y, x, activePuyo.GetValue(y, x));
						activePuyo.SetValue(y, x, NONE);

						//着地判定されたぷよを消す．本処理は必要に応じて変更する．
						//activePuyo.SetValue(y, x, NONE);
					}
				}
			}
		}

		return landed;
	}

	//左移動
	void MoveLeft(PuyoArrayActive &activePuyo)
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

				if (0 < x && activePuyo.GetValue(y, x - 1) == NONE)
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
	void MoveRight(PuyoArrayActive &activePuyo)
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

				if (x < activePuyo.GetColumn() - 1 && activePuyo.GetValue(y, x + 1) == NONE)
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
	void MoveDown(PuyoArray &activePuyo)
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

				if (y < activePuyo.GetLine() - 1 && activePuyo.GetValue(y + 1, x) == NONE)
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
			control.MoveLeft(activePuyo);
			break;
		case KEY_RIGHT:
			control.MoveRight(activePuyo);
			break;
		case 'z':
			//ぷよ回転処理
			break;
		default:
			break;
		}


		//処理速度調整のためのif文
		if (delay%waitCount == 0){
			//ぷよ下に移動
			control.MoveDown(activePuyo);

			//ぷよ着地判定
			if (control.LandingPuyo(activePuyo, stackedPuyo))
			{
				//着地していたら新しいぷよ生成
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

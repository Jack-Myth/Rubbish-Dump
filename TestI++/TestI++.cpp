#include <stdio.h>
#include <windows.h>

int main()
{
	int D[8][8];
	int y = 0;

	D[0][1] = 300; D[0][2] = 360; D[0][3] = 210; D[0][4] = 590; D[0][5] = 475; D[0][6] = 500; D[0][7] = 690;
	D[1][0] = 300; D[1][2] = 380; D[1][3] = 270; D[1][4] = 230; D[1][5] = 285; D[1][6] = 200; D[1][7] = 390;
	D[2][0] = 360; D[2][1] = 380; D[2][3] = 510; D[2][4] = 230; D[2][5] = 765; D[2][6] = 580; D[2][7] = 770;
	D[3][0] = 210; D[3][1] = 270; D[3][2] = 510; D[3][4] = 470; D[3][5] = 265; D[3][6] = 450; D[3][7] = 640;
	D[4][0] = 590; D[4][1] = 230; D[4][2] = 230; D[4][3] = 470; D[4][5] = 515; D[4][6] = 260; D[4][7] = 450;
	D[5][0] = 475; D[5][1] = 285; D[5][2] = 765; D[5][3] = 265; D[5][4] = 515; D[5][6] = 460; D[5][7] = 650;
	D[6][0] = 500; D[6][1] = 200; D[6][2] = 580; D[6][3] = 450; D[6][4] = 260; D[6][5] = 460; D[6][7] = 190;
	D[7][0] = 690; D[7][1] = 390; D[7][2] = 760; D[7][3] = 640; D[7][4] = 450; D[7][5] = 650; D[7][6] = 190;

	y = D[0][3];
	y += D[3][5];
	y += D[5][1];
	y += D[1][6];
	y += D[6][4];
	y += D[4][2];
	y += D[2][7];
	printf("%d\n", y);
	scanf("%*d");
}
#include <iostream>
#include <stdio.h>
#include "stdlib.h"
#include "time.h"
#include <math.h>
#include <random>

using namespace std;

/* S-box */
const int S[16] =
{0Xc, 0xa, 0xd, 0x3, 0xe, 0xb, 0xf, 0x7, 0x8, 0x9, 0x1, 0x5, 0x0, 0x2, 0x4, 0x6};

/* Cell Shuffle in the linear layer */
const int P[16] =
{0xf, 0xc, 0xd, 0xe, 0xa, 0x9, 0x8, 0xb, 0x6, 0x5, 0x4, 0x7, 0x1, 0x2, 0x3, 0x0};

/* Cell Shuffle in the tk schedule */
const int Q[16] =
{0xc, 0xa, 0xf, 0x5, 0xe, 0x8, 0x9, 0x2, 0xb, 0x3, 0x7, 0x4, 0x6, 0x0, 0x1, 0xd};

/* Round constants generated by 3-bit LFSR, XOR-ed at state[5] */
const int RC3[32] =
{0x1, 0x4, 0x2, 0x5, 0x6, 0x7, 0x3, 0x1, 0x4, 0x2, 0x5, 0x6, 0x7, 0x3, 0x1, 0x4,
 0x2, 0x5, 0x6, 0x7, 0x3, 0x1, 0x4, 0x2, 0x5, 0x6, 0x7, 0x3, 0x1, 0x4, 0x2, 0x5};

/* Round constants generated by 4-bit LFSR, XOR-ed at state[4] */
const int RC4 [32] =
{0x1, 0x8, 0x4, 0x2, 0x9, 0xc, 0x6, 0xb, 0x5, 0xa, 0xd, 0xe, 0xf, 0x7, 0x3, 0x1,
 0x8, 0x4, 0x2, 0x9, 0xc, 0x6, 0xb, 0x5, 0xa, 0xd, 0xe, 0xf, 0x7, 0x3, 0x1, 0x8};


int TK[4][16]; /* TK[0][], TK[1][], TK[2][], TK[3][] */


void Initialize_key(int Key[2][16], int Tweak[16]){

    for (int i = 0; i < 16; i++){
        TK[0][i] = Key[0][i] ^ Tweak[i];
        TK[1][i] = Key[1][i] ^ Tweak[i];
        TK[2][i] = Key[0][i] ^ Tweak[Q[i]];
        TK[3][i] = Key[1][i] ^ Tweak[Q[i]];
    }
}


int Round(int Stt[16], int r)
{
    /* MC */
    for (int i = 0; i < 4; i++){
        Stt[i] ^= (Stt[i + 8] ^ Stt[i + 12]);
        Stt[i + 4] ^= Stt[i + 12];
    }

    /* Add constants */
    Stt[4] ^= RC4[r];
    Stt[5] ^= RC3[r];

    /* Add tweakey */
    for (int i = 0; i < 16; i++)
        Stt[i] ^= TK[r%4][i];


    int Temp[16];

    /* PN */
    for (int i = 0; i < 16; i++)
        Temp[P[i]] = Stt[i];

    /* SB */
    for (int i = 0; i < 16; i++)
        Stt[i] = S[Temp[i]];

}

int Last_round(int Stt[16], int r)
{
    /* MC */
    for (int i = 0; i < 4; i++){
        Stt[i] ^= (Stt[i + 8] ^ Stt[i + 12]);
        Stt[i + 4] ^= Stt[i + 12];
    }

    /* Add constants */
    Stt[4] ^= RC4[r];
    Stt[5] ^= RC3[r];

    /* Add tweakey */
    for (int i = 0; i < 16; i++)
        Stt[i] ^= TK[r%4][i];
}



int Test_Condition(int C[16])
{
     if (C[ 0] == 0x0 and C[ 1] == 0x0 and C[ 2] == 0xa and C[ 3] == 0x0 and
         C[ 4] == 0xa and                  C[ 6] == 0x0 and C[ 7] == 0x0 and
         C[ 8] == 0x0 and                  C[10] == 0x0 and C[11] == 0x0 and
                                           C[14] == 0x0 and C[15] == 0x0)
     {
          return 1;
     }
     else return 0;
}


int main()
{
    int Key[2][16];
    int Tweak[16];
    int P1[16];
    int P0[16];
    int Diff_Out[16];

    int Diff[16] =
    {
        0x0, 0x0, 0xa, 0x0,
        0xa, 0x0, 0x0, 0x0,
        0x0, 0xa, 0x0, 0x0,
        0x0, 0xa, 0x0, 0x0
    };

    int R0 = 10;              /* Number of rounds */
    double d = pow(2, 32);      /* Number of pairs */

    double Pr;
    double Pr_sum = 0;

    cout << "data : " ;
    cout << d << endl ;

    cout << "Round : " ;
    cout << R0 <<endl;

    mt19937_64 mt_rand(time(0));

    for(int k = 0; k < 10; k++)
    {
        double counter_for_correct_pairs = 0;
        cout << "Key: " << endl;
        for (int i = 0; i < 16; i++)
        {
            Key[0][i] = mt_rand()%16;
            cout << hex << Key[0][i];
        }

        for (int i = 0; i < 16; i++)
        {
            Key[1][i] = mt_rand()%16;
            cout << hex << Key[1][i];
        }
        cout << endl;

        for (double data = 0; data < d; data++)
        {
            for (int i = 0; i < 16; i++)
                P0[i] = mt_rand()%16;

            for (int i = 0; i < 16; i++)
                P1[i] = P0[i] ^ Diff[i];

            for (int i = 0; i < 16; i++)
                Tweak[i] = mt_rand()%16;

            Tweak[10] = Key[1][10]; Tweak[7] = Key[1][10];
            Tweak[13] = Key[1][13]; Tweak[0] = Key[1][13];
            Tweak[9] = Key[1][9]; Tweak[3] = Key[1][9];

            Initialize_key(Key, Tweak);

            for (int r = 0; r < R0; r++)
                Round(P0, r);

            for (int r = 0; r<R0; r++)
                Round(P1, r);

            for (int i = 0; i < 16; i++)
                Diff_Out[i] = P0[i] ^ P1[i];

            if (Test_Condition(Diff_Out))
                counter_for_correct_pairs = counter_for_correct_pairs + 1;
        }

        Pr = float(counter_for_correct_pairs)/float(d);
        Pr_sum = Pr_sum + Pr;

        cout << "number of right pairs : " ;
        cout << counter_for_correct_pairs << endl;

        cout << "pro : " ;
        cout << log2 (Pr) << endl;

    }
    cout << "average pro: " << log2(Pr_sum/10);

    return 0;
}






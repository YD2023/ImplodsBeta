#include <string.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#ifdef  WIN64
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

#define GET_BIT(bitboard, square) ((bitboard) & (1ULL << (square)))
#define SET_BIT(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define POP_BIT(bitboard, square) (GET_BIT(bitboard, square) ? (bitboard) ^= (1ULL<< square) : 0)
#define COUNT_BITS(bitboard) __builtin_popcountll(bitboard)
#define empty_board "8/8/8/8/8/8/8/8 b - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "
#define ENCODE_MOVE(source, target, piece, promoted, capture, double, enpassant, castling) \
    (source) | (target <<6) | (piece << 12) | \
    (promoted <<16) | (capture << 20) | (double << 21) | \
    (enpassant << 22) | (castling << 23)

#define GET_MOVE_SOURCE(move) (move & 0x3f)
#define GET_MOVE_TARGET(move) ((move & 0xfc0) >> 6)
#define GET_MOVE_PIECE(move) ((move & 0xf000) >> 12)
#define GET_MOVE_PROMOTED(move) ((move & 0xf0000) >> 16)
#define GET_MOVE_CAPTURE_FLAG(move) (move & 0x100000)
#define GET_MOVE_DOUBLE_PUSH_FLAG(move) (move & 0x200000)
#define GET_MOVE_ENPASSANT_FLAG(move) (move & 0x400000)
#define GET_MOVE_CASTLING_FLAG(move) (move & 0x800000)

int time_check(){
    #ifdef WIN64
        return GetTickCount();
    #else
        struct timeval time_val;
        gettimeofday(&time_val, NULL);
        return (time_val.tv_sec * 1000) + (time_val.tv_usec / 1000);
    #endif
}

static inline int GET_INDEX_OF_LSB1(uint64_t bitboard) {
    return bitboard ? __builtin_ffsll(bitboard) - 1 : -1;
}

// Constants for the bit positions of each square
enum square {
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1, no_square

};
const char *square_to_coords[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "no_square"
};

//Relevant Occupancy Bit Count across Board
const int bishop_rel_bit[64] = {
6, 5, 5, 5, 5, 5, 5, 6, 
5, 5, 5, 5, 5, 5, 5, 5, 
5, 5, 7, 7, 7, 7, 5, 5, 
5, 5, 7, 9, 9, 7, 5, 5, 
5, 5, 7, 9, 9, 7, 5, 5, 
5, 5, 7, 7, 7, 7, 5, 5, 
5, 5, 5, 5, 5, 5, 5, 5, 
6, 5, 5, 5, 5, 5, 5, 6,
};

const int rook_rel_bit[64] = {
12, 11, 11, 11, 11, 11, 11, 12, 
11, 10, 10, 10, 10, 10, 10, 11, 
11, 10, 10, 10, 10, 10, 10, 11, 
11, 10, 10, 10, 10, 10, 10, 11, 
11, 10, 10, 10, 10, 10, 10, 11, 
11, 10, 10, 10, 10, 10, 10, 11, 
11, 10, 10, 10, 10, 10, 10, 11, 
12, 11, 11, 11, 11, 11, 11, 12, 
};

//sides to move
enum {white, black, both};
enum {rook, bishop};

//Castling constants
enum {WK = 1 , WQ = 2, BK = 4, BQ = 8};

//encoding pieces (Upper case is white, lower case is black)
enum{P, N, B, R, Q, K, p, n, b, r, q, k};

//ASCII pieces to encoded pieces coversion
char ascII[13]= "PNBRQKpnbrqk";
//unicode pieces
//char *unicode[12] = {"♟︎","♞","♝","♜","♛","♚", "♙","♘","♗","♖", "♕","♔"};
int char_pieces[128] = {0};
char promoted_pieces[12] = {0};

// Initialize arrays
void initialize_arrays() {
    char_pieces['P'] = P;
    char_pieces['N'] = N;
    char_pieces['B'] = B;
    char_pieces['R'] = R;
    char_pieces['Q'] = Q;
    char_pieces['K'] = K;
    char_pieces['p'] = p;
    char_pieces['n'] = n;
    char_pieces['b'] = b;
    char_pieces['r'] = r;
    char_pieces['q'] = q;
    char_pieces['k'] = k;

    promoted_pieces[Q] = 'q';
    promoted_pieces[R] = 'r';
    promoted_pieces[B] = 'b';
    promoted_pieces[N] = 'n';
    promoted_pieces[q] = 'q';
    promoted_pieces[r] = 'r';
    promoted_pieces[b] = 'b';
    promoted_pieces[n] = 'n';
}

//const uint64_t FILE_A = 0x0101010101010101ULL;
//const uint64_t FILE_H = 0x8080808080808080ULL;
static const uint64_t notAFile = 0xFEFEFEFEFEFEFEFEULL;
static const uint64_t notABFile = 0xFCFCFCFCFCFCFCFCULL;
static const uint64_t notHFile = 0x7F7F7F7F7F7F7F7FULL;
static const uint64_t notGHFile = 0x3F3F3F3F3F3F3F3FULL;

uint64_t rook_magic_nums[64] = {
    0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL, 0x100081001000420ULL,
    0x200020010080420ULL, 0x3001c0002010008ULL, 0x8480008002000100ULL, 0x2080088004402900ULL,
    0x800098204000ULL, 0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL,
    0x208808088000400ULL, 0x2802200800400ULL, 0x2200800100020080ULL, 0x801000060821100ULL,
    0x80044006422000ULL, 0x100808020004000ULL, 0x12108a0010204200ULL, 0x140848010000802ULL,
    0x481828014002800ULL, 0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
    0x100400080208000ULL, 0x2040002120081000ULL, 0x21200680100081ULL, 0x20100080080080ULL,
    0x2000a00200410ULL, 0x20080800400ULL, 0x80088400100102ULL, 0x80004600042881ULL,
    0x4040008040800020ULL, 0x440003000200801ULL, 0x4200011004500ULL, 0x188020010100100ULL,
    0x14800401802800ULL, 0x2080040080800200ULL, 0x124080204001001ULL, 0x200046502000484ULL,
    0x480400080088020ULL, 0x1000422010034000ULL, 0x30200100110040ULL, 0x100021010009ULL,
    0x2002080100110004ULL, 0x202008004008002ULL, 0x20020004010100ULL, 0x2048440040820001ULL,
    0x101002200408200ULL, 0x40802000401080ULL, 0x4008142004410100ULL, 0x2060820c0120200ULL,
    0x1001004080100ULL, 0x20c020080040080ULL, 0x2935610830022400ULL, 0x44440041009200ULL,
    0x280001040802101ULL, 0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
    0x20030a0244872ULL, 0x12001008414402ULL, 0x2006104900a0804ULL, 0x1004081002402ULL};

uint64_t bishop_magic_nums[64] = {
    0x40040844404084ULL, 0x2004208a004208ULL, 0x10190041080202ULL, 0x108060845042010ULL,
    0x581104180800210ULL, 0x2112080446200010ULL, 0x1080820820060210ULL, 0x3c0808410220200ULL,
    0x4050404440404ULL, 0x21001420088ULL, 0x24d0080801082102ULL, 0x1020a0a020400ULL,
    0x40308200402ULL, 0x4011002100800ULL, 0x401484104104005ULL, 0x801010402020200ULL,
    0x400210c3880100ULL, 0x404022024108200ULL, 0x810018200204102ULL, 0x4002801a02003ULL,
    0x85040820080400ULL, 0x810102c808880400ULL, 0xe900410884800ULL, 0x8002020480840102ULL,
    0x220200865090201ULL, 0x2010100a02021202ULL, 0x152048408022401ULL, 0x20080002081110ULL,
    0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL, 0x1c004001012080ULL,
    0x8004200962a00220ULL, 0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
    0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL,
    0x209188240001000ULL, 0x400408a884001800ULL, 0x110400a6080400ULL, 0x1840060a44020800ULL,
    0x90080104000041ULL, 0x201011000808101ULL, 0x1a2208080504f080ULL, 0x8012020600211212ULL,
    0x500861011240000ULL, 0x180806108200800ULL, 0x4000020e01040044ULL, 0x300000261044000aULL,
    0x802241102020002ULL, 0x20906061210001ULL, 0x5a84841004010310ULL, 0x4010801011c04ULL,
    0xa010109502200ULL, 0x4a02012000ULL, 0x500201010098b028ULL, 0x8040002811040900ULL,
    0x28000010020204ULL, 0x6000020202d0240ULL, 0x8918844842082200ULL, 0x4010011029020020ULL};

    //New pawn functions
    uint64_t pawn_attacks[2][64];
    uint64_t knight_attacks[64];
    uint64_t king_attacks[64];
    uint64_t bishop_attacks[64][512];
    uint64_t rook_attacks[64][4096];
    uint64_t bishop_masks[64];
    uint64_t rook_masks[64];

/*
-----------------
Setting up the board with castle rules and defined boards
------------------
*/


    //Setting up the constants board
    uint64_t bitboards[12];
    uint64_t occupanicies[3];
    int side;
    int enpassant = no_square;
    int castle;
    uint64_t bitboards_copy[12], occupanicies_copy[3]; 
    int side_copy, enpassant_copy, castle_copy; 

    #define copy_board() \
        memcpy(bitboards_copy, bitboards, sizeof(bitboards)); \
        memcpy(occupanicies_copy, occupanicies, sizeof(occupanicies)); \
        side_copy = side; \
        enpassant_copy = enpassant; \
        castle_copy = castle; \

    #define restore_board() \
        memcpy(bitboards, bitboards_copy, 96); \
        memcpy(occupanicies, occupanicies_copy, 24); \
        side = side_copy; \
        enpassant = enpassant_copy; \
        castle = castle_copy; \

static inline uint64_t get_bishop_attacks(int square, uint64_t occupancy)
{
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_nums[square];
    occupancy >>= (64 - bishop_rel_bit[square]);
    return bishop_attacks[square][occupancy];
}

static inline uint64_t get_rook_attacks(int square, uint64_t occupancy)
{
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_nums[square];
    occupancy >>= (64 - rook_rel_bit[square]);
    return rook_attacks[square][occupancy];
}

static inline uint64_t get_queen_attacks(int square, uint64_t occupancy){
    uint64_t result = 0ULL;
    uint64_t bishop_occupancies = occupancy;
    uint64_t rook_occupancies = occupancy;

    bishop_occupancies &= bishop_masks[square];
    bishop_occupancies *= bishop_magic_nums[square];
    bishop_occupancies >>= (64 - bishop_rel_bit[square]);
    result = bishop_attacks[square][bishop_occupancies];

    rook_occupancies &= rook_masks[square];
    rook_occupancies *= rook_magic_nums[square];
    rook_occupancies >>= (64 - rook_rel_bit[square]);
    result |= rook_attacks[square][rook_occupancies];

    return result;
}

// generate pawn attacks
uint64_t mask_pawn_attacks(int side, int square)
{
    // Result attacks bitboard
    uint64_t attacks = 0ULL;

    // piece bitboard
    uint64_t bitboard = 0ULL;

    // Set the piece on Board
    SET_BIT(bitboard, square);

    // White pawns
    if (!side)
    {
        if ((bitboard >> 7) & notAFile)
            attacks |= (bitboard >> 7);
        if ((bitboard >> 9) & notHFile)
            attacks |= (bitboard >> 9);
    }
    else
    {
        if ((bitboard << 7) & notHFile)
            attacks |= (bitboard << 7);
        if ((bitboard << 9) & notAFile)
            attacks |= (bitboard << 9);
    }

    // return attack map
    return attacks;
}



// generate knight attacks
uint64_t mask_knight_attacks(int square)
{
    // Result attacks bitboard
    uint64_t attacks = 0ULL;

    // piece bitboard
    uint64_t bitboard = 0ULL;

    // Set the piece on Board
    SET_BIT(bitboard, square);

    // generate knight attacks
    if ((bitboard >> 17) & notHFile)
        attacks |= (bitboard >> 17);
    if ((bitboard >> 15) & notAFile)
        attacks |= (bitboard >> 15);
    if ((bitboard >> 10) & notGHFile)
        attacks |= (bitboard >> 10);
    if ((bitboard >> 6) & notABFile)
        attacks |= (bitboard >> 6);

    if ((bitboard << 17) & notAFile)
        attacks |= (bitboard << 17);
    if ((bitboard << 15) & notHFile)
        attacks |= (bitboard << 15);
    if ((bitboard << 10) & notABFile)
        attacks |= (bitboard << 10);
    if ((bitboard << 6) & notGHFile)
        attacks |= (bitboard << 6);

    // returning the attack map
    return attacks;
}

uint64_t mask_king_attacks(int square)
{
    // Result attacks bitboard
    uint64_t attacks = 0ULL;

    // piece bitboard
    uint64_t bitboard = 0ULL;

    // Set the piece on Board
    SET_BIT(bitboard, square);

    // generate king moves
    if (bitboard >> 8)
        attacks |= (bitboard >> 8);
    if ((bitboard >> 9) & notHFile)
        attacks |= (bitboard >> 9);
    if ((bitboard >> 7) & notAFile)
        attacks |= (bitboard >> 7);
    if ((bitboard >> 1) & notHFile)
        attacks |= (bitboard >> 1);
    if (bitboard << 8)
        attacks |= (bitboard << 8);
    if ((bitboard << 9) & notAFile)
        attacks |= (bitboard << 9);
    if ((bitboard << 7) & notHFile)
        attacks |= (bitboard << 7);
    if ((bitboard << 1) & notAFile)
        attacks |= (bitboard << 1);

    // returning the attack map
    return attacks;
}
// Initializing leaper pieces Attacks
void init_leapers_attacks()
{
    // Looping through bitboard squares
    for (int square = 0; square < 64; square++)
    {
        // init pawn attacks
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);

        // init knight attacks
        knight_attacks[square] = mask_knight_attacks(square);

        // init king attacks
        king_attacks[square] = mask_king_attacks(square);
    }
}
uint64_t mask_bishop_attacks(int square)
{
    // result attacks bitboard
    uint64_t attacks = 0ULL;

    // init ranks and files
    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    // mask relevant bishop occupancy bits
    for (rank = targetRank + 1, file = targetFile + 1; rank <= 6 && file <= 6; rank++, file++)
    {
        attacks |= (1ULL << (rank * 8 + file));
    }
    for (rank = targetRank - 1, file = targetFile + 1; rank >= 1 && file <= 6; rank--, file++)
    {
        attacks |= (1ULL << (rank * 8 + file));
    }
    for (rank = targetRank + 1, file = targetFile - 1; rank <= 6 && file >= 1; rank++, file--)
    {
        attacks |= (1ULL << (rank * 8 + file));
    }
    for (rank = targetRank - 1, file = targetFile - 1; rank >= 1 && file >= 1; rank--, file--)
    {
        attacks |= (1ULL << (rank * 8 + file));
    }
    return attacks;
}

uint64_t bishop_attacks_wfb(int square, uint64_t blockbitboard)
{
    // result attacks bitboard
    uint64_t attacks = 0ULL;

    // init ranks and files
    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    // generates bishop attacks on the border as well
    for (rank = targetRank + 1, file = targetFile + 1; rank <= 7 && file <= 7; rank++, file++)
    {
        attacks |= (1ULL << (rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blockbitboard)
            break;
    }
    for (rank = targetRank - 1, file = targetFile + 1; rank >= 0 && file <= 7; rank--, file++)
    {
        attacks |= (1ULL << (rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blockbitboard)
            break;
    }
    for (rank = targetRank + 1, file = targetFile - 1; rank <= 7 && file >= 0; rank++, file--)
    {
        attacks |= (1ULL << (rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blockbitboard)
            break;
    }
    for (rank = targetRank - 1, file = targetFile - 1; rank >= 0 && file >= 0; rank--, file--)
    {
        attacks |= (1ULL << (rank * 8 + file));
        if ((1ULL << (rank * 8 + file)) & blockbitboard)
            break;
    }
    return attacks;
}

uint64_t mask_rook_attacks(int square)
{
    // result attacks bitboard
    uint64_t attacks = 0ULL;

    // init ranks and files
    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    // mask rook squares
    for (rank = targetRank + 1; rank <= 6; rank++)
    {
        attacks |= (1Ull << (rank * 8 + targetFile));
    }
    for (rank = targetRank - 1; rank >= 1; rank--)
    {
        attacks |= (1Ull << (rank * 8 + targetFile));
    }
    for (file = targetFile + 1; file <= 6; file++)
    {
        attacks |= (1Ull << (targetRank * 8 + file));
    }
    for (file = targetFile - 1; file >= 1; file--)
    {
        attacks |= (1Ull << (targetRank * 8 + file));
    }

    return attacks;
}

uint64_t rook_attacks_wfb(int square, uint64_t blockbitboard)
{
    // result attacks bitboard
    uint64_t attacks = 0ULL;

    // init ranks and files
    int rank, file;
    int targetRank = square / 8;
    int targetFile = square % 8;

    // mask rook squares
    for (rank = targetRank + 1; rank <= 7; rank++)
    {
        attacks |= (1Ull << (rank * 8 + targetFile));
        if (1Ull << (rank * 8 + targetFile) & blockbitboard)
            break;
    }
    for (rank = targetRank - 1; rank >= 0; rank--)
    {
        attacks |= (1Ull << (rank * 8 + targetFile));
        if (1Ull << (rank * 8 + targetFile) & blockbitboard)
            break;
    }
    for (file = targetFile + 1; file <= 7; file++)
    {
        attacks |= (1Ull << (targetRank * 8 + file));
        if (1Ull << (targetRank * 8 + file) & blockbitboard)
            break;
    }
    for (file = targetFile - 1; file >= 0; file--)
    {
        attacks |= (1Ull << (targetRank * 8 + file));
        if (1Ull << (targetRank * 8 + file) & blockbitboard)
            break;
    }

    return attacks;
}


// Setting up occupancy tables
uint64_t set_occupy(int index, int maskbits, uint64_t mask)
{
    // occupancy map
    uint64_t occupancy = 0ULL;
    for (int x = 0; x < maskbits; ++x)
    {
        int square = GET_INDEX_OF_LSB1(mask);
        POP_BIT(mask, square);
        if (index & (1 << x))
        {
            occupancy |= (1ULL << square);
        }
    }

    return occupancy;
}

// pseudo random number generator XORSHIFT32 with 32-bit numbers
unsigned int Rstate = 1804289383;
unsigned int get_rand_U32num()
{
    unsigned int num = Rstate;
    num ^= num << 13;
    num ^= num >> 17;
    num ^= num << 5;

    Rstate = num;

    return num;
}

// Converting to unisgned 64 bit numbers
uint64_t get_rand_U64num()
{
    uint64_t n1, n2, n3, n4;

    n1 = uint64_t(get_rand_U32num()) & 0xFFFF;
    n2 = uint64_t(get_rand_U32num()) & 0xFFFF;
    n3 = uint64_t(get_rand_U32num()) & 0xFFFF;
    n4 = uint64_t(get_rand_U32num()) & 0xFFFF;

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// Generating magic numbers
uint64_t generate_magic_numbers()
{
    return get_rand_U64num() & get_rand_U64num() & get_rand_U64num();
}

// Magic Number finding and handling
uint64_t find_magic(int square, int relevant_bits, int bishop)
{
    // occupancies
    uint64_t occupancy[4096];

    // initializing the attack tables and used attacks
    uint64_t attacks[4096];
    uint64_t used[4096];

    // initializing mask for current piece
    uint64_t mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);
    int occupancy_index = 1 << relevant_bits;

    // loop over occupancy index
    for (int index = 0; index < occupancy_index; index++)
    {
        // initializing occupancy
        occupancy[index] = set_occupy(index, relevant_bits, mask);
        attacks[index] = bishop ? bishop_attacks_wfb(square, occupancy[index]) : rook_attacks_wfb(square, occupancy[index]);
    }
    // testing magic numbers
    for (int count = 0; count < 100000000; count++)
    {
        // gen candidates
        uint64_t magic_number = generate_magic_numbers();

        // skip innapropriate magic num
        if (COUNT_BITS((mask * magic_number) & 0xFF00000000000000) < 6)
            continue;

        memset(used, 0ULL, sizeof(used));

        // init index and fail flag
        int index, fail;

        for (index = 0, fail = 0; !fail && index < occupancy_index; index++)
        {
            int magic_index = (int)((occupancy[index] * magic_number) >> (64 - relevant_bits));

            if (used[magic_index] == 0ULL)
            {
                used[magic_index] = attacks[index];
            }
            else if (used[magic_index] != attacks[index])
            {
                fail = 1;
            }
        }
        if (!fail)
        {
            return magic_number;
        }
    }
    printf("Magic number failed");
    return 0ULL;
}

void init_magic_numbers()
{
    for (int square = 0; square < 64; square++)
    {
        rook_magic_nums[square] = find_magic(square, rook_rel_bit[square], rook);
    }
    for (int square = 0; square < 64; square++)
    {
        bishop_magic_nums[square] = find_magic(square, bishop_rel_bit[square], bishop);
    }
}

// initializing slider pieces' attack tables
void init_slider_attack_tables(int bishop)
{
    // initializing bishop and rook masks
    for (int square = 0; square < 64; square++)
    {
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);

        // current mask and occupancy
        uint64_t current_attack_mask = bishop ? bishop_masks[square] : rook_masks[square];
        int relevant_bits = COUNT_BITS(current_attack_mask);
        int occupancy_index = 1 << relevant_bits;

        for (int index = 0; index < occupancy_index; index++)
        {
            if (bishop)
            {
                uint64_t occupancy = set_occupy(index, relevant_bits, current_attack_mask);
                int magic_index = (occupancy * bishop_magic_nums[square]) >> (64 - bishop_rel_bit[square]);
                bishop_attacks[square][magic_index] = bishop_attacks_wfb(square, occupancy);
            }
            else
            {
                uint64_t occupancy = set_occupy(index, relevant_bits, current_attack_mask);
                int magic_index = (occupancy * rook_magic_nums[square]) >> (64 - rook_rel_bit[square]);
                rook_attacks[square][magic_index] = rook_attacks_wfb(square, occupancy);
            }
        }
    }
}

void init_all()
{
    // init leaper attacks
    initialize_arrays();
    init_leapers_attacks();
    init_slider_attack_tables(bishop);
    init_slider_attack_tables(rook);
    // init_magic_numbers();
}


// Function to print the bitboard in a readable format
void print_bitboard(uint64_t bitboard)
{
    for (int rank = 0; rank < 8; ++rank)
    {
        for (int file = 0; file < 8; ++file)
        {
            int square = rank * 8 + file;
            if (!file)
            {
                std::cout << 8 - rank << " ";
            }
            std::cout << (GET_BIT(bitboard, square) ? '1' : '0') << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << "  a b c d e f g h\n\n";

    std::cout << "Bitboard: " << bitboard << "\n\n";
}

//print chess board with pieces
void print_board(){
    printf("\n");
    for (int rank = 0; rank < 8; ++rank)
    {
        printf(" %d ", 8 - rank);
        for (int file = 0; file < 8; ++file)
        {
            int square = rank * 8 +file;
            int piece = -1;

            for (int bb = P; bb<= k ; bb++){
                if (GET_BIT(bitboards[bb], square)){ piece = bb; }
            }

            printf(" %c", (piece ==-1) ? '.': ascII[piece]);
            //printf(" %s", (piece ==-1) ? '.': unicode[piece]);
        }
        std::cout<<"\n";
    }
    printf("\n    a b c d e f g h\n\n");
    printf("Side to move: %s\n", (!side) ? "white" :" black");
    printf("EnPassant: %s\n", (enpassant != no_square) ? square_to_coords[enpassant] : "No");
    printf("Castling Rights: %c%c%c%c\n", (castle & WK) ? 'K': '-', (castle & WQ) ? 'Q': '-', 
                                            (castle & BK) ? 'k': '-', (castle & BQ) ? 'q': '-');

}
void FEN_parse(const char *fen) {
    // Resetting the board
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupanicies, 0ULL, sizeof(occupanicies));
    side = 0;
    castle = 0;
    enpassant = no_square;
    
    // Parse piece placement
    for (int square = 0; square < 64 && *fen && *fen != ' '; ) {
        if ((*fen >= 'b' && *fen <= 'r') || (*fen >= 'B' && *fen <= 'R')) {
            int piece = char_pieces[*fen];
            SET_BIT(bitboards[piece], square);

            square++;
            fen++;
        }
        else if (*fen >= '1' && *fen <= '8') {
            int offset = *fen - '0';
            square += offset;
            fen++;
        }
        else if (*fen == '/') {
            fen++;
        }
        else {
            fen++;
        }
    }

    // Skip space
    fen++;

    // Parse side to move
    side = (*fen == 'w') ? white : black;
    fen += 2;

    // Parse castling rights
    while (*fen != ' ') {
        switch (*fen) {
            case 'K': castle |= WK; break;
            case 'Q': castle |= WQ; break;
            case 'k': castle |= BK; break;
            case 'q': castle |= BQ; break;
            case '-': break;
        }
        fen++;
    }
    fen++;

    // Parse en passant target square
    if (*fen != '-') {
        int file = fen[0] - 'a';
        int rank = 8 - (fen[1] - '0');
        enpassant = rank * 8 + file;
    }
    fen += 2;

    // Skip halfmove clock and fullmove number
    // These are not used in this function but can be parsed if needed
    // sscanf(fen, "%d %d", &halfmove_clock, &fullmove_number);

    // Update occupancies
    for (int bb_piece = P; bb_piece <= K; bb_piece++) occupanicies[white] |= bitboards[bb_piece];
    for (int bb_piece = p; bb_piece <= k; bb_piece++) occupanicies[black] |= bitboards[bb_piece];
    occupanicies[both] = occupanicies[white] | occupanicies[black];
}

//MOVE DECLARATION AND MOVE LIST
//move list
struct moves {
    int moves_array[256]; // Array to store moves
    int count;      // Move count

    moves() : count(0) {} // Constructor to initialize count to 0
};

static inline void add_move(moves *move_list, int move){
    move_list -> moves_array[move_list->count] = move;
    move_list->count++;
}



//print move
void print_move(int move){
    if (GET_MOVE_PROMOTED(move)){
    printf("%s%s%c", square_to_coords[GET_MOVE_SOURCE(move)],
                    square_to_coords[GET_MOVE_TARGET(move)], 
                    promoted_pieces[GET_MOVE_PROMOTED(move)]);
    }
    else{
            printf("%s%s", square_to_coords[GET_MOVE_SOURCE(move)],
                    square_to_coords[GET_MOVE_TARGET(move)]);
    }

}
void print_move_list(moves *move_list){

    if (!move_list->count){
        std::cout<< "No move in move list"<<std::endl;
        return;
    }

    printf("\n    move   piece   capture   double   enpassant   castling\n\n");
    for (int move_count = 0; move_count< (move_list->count); move_count++){
        int move = move_list -> moves_array[move_count];
    printf("    %s%s%c    %c     %d         %d         %d          %d\n", square_to_coords[GET_MOVE_SOURCE(move)],
                    square_to_coords[GET_MOVE_TARGET(move)], 
                    promoted_pieces[GET_MOVE_PROMOTED(move)],
                    ascII[GET_MOVE_PIECE(move)],
                    GET_MOVE_CAPTURE_FLAG(move) ? 1: 0,
                    GET_MOVE_DOUBLE_PUSH_FLAG(move) ? 1: 0,
                    GET_MOVE_ENPASSANT_FLAG(move) ? 1: 0,
                    GET_MOVE_CASTLING_FLAG(move) ? 1: 0);
    }
    printf("\n\nTotal Number of Moves: %d", move_list->count);

}


//Checking if a square is attacked
static inline int square_attacked(int square, int side) {
    // Pawn attacks
    if (side == white) {
        if (pawn_attacks[black][square] & bitboards[P]) return 1;
        if (knight_attacks[square] & bitboards[N]) return 1;
        if (king_attacks[square] & bitboards[K]) return 1;
        if (get_bishop_attacks(square, occupanicies[both]) & (bitboards[B] | bitboards[Q])) return 1;
        if (get_rook_attacks(square, occupanicies[both]) & (bitboards[R] | bitboards[Q])) return 1;

    } else {
        if (pawn_attacks[white][square] & bitboards[p]) return 1;
        if (knight_attacks[square] & bitboards[n]) return 1;
        if (king_attacks[square] & bitboards[k]) return 1;
        if (get_bishop_attacks(square, occupanicies[both]) & (bitboards[b] | bitboards[q])) return 1;
        if (get_rook_attacks(square, occupanicies[both]) & (bitboards[r] | bitboards[q])) return 1;

    }

    return 0;
}

// MOVE GENERATOR
// Generating moves and move lists
static inline void generate_moves(moves *move_list) {
    // Init move count
    move_list->count = 0;

    int source_square, target_square;
    uint64_t bitboard, attacks; 
    for (int piece = (side == white ? P : p); piece <= (side == white ? K : k); piece++) {
        bitboard = bitboards[piece];
        if (side == white)
        {
            if (piece == P)
            {
                while (bitboard)
                {
                    // init source square
                    source_square = GET_INDEX_OF_LSB1(bitboard);
                    
                    // init target square
                    target_square = source_square - 8;
                    
                    if (!(target_square < A8) && !GET_BIT(occupanicies[both], target_square))
                    {
                        if (source_square >= A7 && source_square <= H7)
                        {                            
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, Q, 0, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, R, 0, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, B, 0, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, N, 0, 0, 0, 0));
                        }
                        
                        else
                        {
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                            if ((source_square >= A2 && source_square <= H2) && !GET_BIT(occupanicies[both], (target_square - 8)))
                                add_move(move_list, ENCODE_MOVE(source_square, (target_square - 8), piece, 0, 0, 1, 0, 0));
                        }
                    }
                    
                    attacks = pawn_attacks[side][source_square] & occupanicies[black];
                    
                    while (attacks)
                    {
                        // init target square
                        target_square = GET_INDEX_OF_LSB1(attacks);
                        
                        // pawn promotion
                        if (source_square >= A7 && source_square <= H7)
                        {
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, Q, 1, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, R, 1, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, B, 1, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, N, 1, 0, 0, 0));
                        }
                        
                        else
                            // one square ahead pawn move
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        
                        // pop ls1b of the pawn attacks
                        POP_BIT(attacks, target_square);
                    }
                    
                    // generate enpassant captures
                    if (enpassant != no_square)
                    {
                        uint64_t enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                        
                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = GET_INDEX_OF_LSB1(enpassant_attacks);
                            add_move(move_list, ENCODE_MOVE(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    POP_BIT(bitboard, source_square);
                }
            }
            
            // castling moves
            if (piece == K)
            {
                // king side castling is available
                if (castle & WK)
                {
                    // make sure square between king and king's rook are empty
                    if (!GET_BIT(occupanicies[both], F1) && !GET_BIT(occupanicies[both], G1))
                    {
                        // make sure king and the f1 squares are not under attacks
                        if (!square_attacked(E1, black) && !square_attacked(F1, black))
                            add_move(move_list, ENCODE_MOVE(E1, G1, piece, 0, 0, 0, 0, 1));
                    }
                }
                
                // queen side castling is available
                if (castle & WQ)
                {
                    // make sure square between king and queen's rook are empty
                    if (!GET_BIT(occupanicies[both], D1) && !GET_BIT(occupanicies[both], C1) && !GET_BIT(occupanicies[both], B1))
                    {
                        // make sure king and the d1 squares are not under attacks
                        if (!square_attacked(E1, black) && !square_attacked(D1, black))
                            add_move(move_list, ENCODE_MOVE(E1, C1, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }
        
        else
        {
            if (piece == p)
            {
                while (bitboard)
                {
                    // init source square
                    source_square = GET_INDEX_OF_LSB1(bitboard);
                    
                    // init target square
                    target_square = source_square + 8;
                    
                    if (!(target_square > H1) && !GET_BIT(occupanicies[both], target_square))
                    {
                        // pawn promotion
                        if (source_square >= A2 && source_square <= H2)
                        {
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, q, 0, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, r, 0, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, b, 0, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, n, 0, 0, 0, 0));
                        }
                        
                        else
                        {
                            // one square ahead pawn move
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                            // double push
                            if ((source_square >= A7 && source_square <= H7) && !GET_BIT(occupanicies[both], (target_square + 8)))
                                add_move(move_list, ENCODE_MOVE(source_square, (target_square + 8), piece, 0, 0, 1, 0, 0));
                        }
                    }
                    
                    attacks = pawn_attacks[side][source_square] & occupanicies[white];
                    
                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        target_square = GET_INDEX_OF_LSB1(attacks);
                        
                        // pawn promotion
                        if (source_square >= A2 && source_square <= H2)
                        {
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, q, 1, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, r, 1, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, b, 1, 0, 0, 0));
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, n, 1, 0, 0, 0));
                        }
                        
                        else
                            // one square ahead pawn move
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        
                        // pop ls1b of the pawn attacks
                        POP_BIT(attacks, target_square);
                    }
                    
                    // generate enpassant captures
                    if (enpassant != no_square)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        uint64_t enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                        
                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = GET_INDEX_OF_LSB1(enpassant_attacks);
                            add_move(move_list, ENCODE_MOVE(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    POP_BIT(bitboard, source_square);
                }
            }
            
            // castling moves
            if (piece == k)
            {
                // king side castling is available
                if (castle & BK)
                {
                    // make sure square between king and king's rook are empty
                    if (!GET_BIT(occupanicies[both], F8) && !GET_BIT(occupanicies[both], G8))
                    {
                        // make sure king and the f8 squares are not under attacks
                        if (!square_attacked(E8, white) && !square_attacked(F8, white))
                            add_move(move_list, ENCODE_MOVE(E8, G8, piece, 0, 0, 0, 0, 1));
                    }
                }
                
                // queen side castling is available
                if (castle & BQ)
                {
                    // make sure square between king and queen's rook are empty
                    if (!GET_BIT(occupanicies[both], D8) && !GET_BIT(occupanicies[both], C8) && !GET_BIT(occupanicies[both], B8))
                    {
                        // make sure king and the d8 squares are not under attacks
                        if (!square_attacked(E8, white) && !square_attacked(D8, white))
                            add_move(move_list, ENCODE_MOVE(E8, C8, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }
        // Generate moves for all pieces of the current type
        while (bitboard) {
            source_square = GET_INDEX_OF_LSB1(bitboard);
            POP_BIT(bitboard, source_square);

            // Generate moves based on piece type
            switch (piece) {
                case N:
                case n:
                    attacks = knight_attacks[source_square] & ((side == white) ? ~occupanicies[white] : ~occupanicies[black]);
                    while (attacks) {
                        target_square = GET_INDEX_OF_LSB1(attacks);
                        POP_BIT(attacks, target_square);
                        if (side == white && GET_BIT(occupanicies[black], target_square)){
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        else if(side == black && GET_BIT(occupanicies[white], target_square)){
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        else{
                        add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0));
                        }
                    }

                    break;
                case B:
                case b:
                    attacks = get_bishop_attacks(source_square, occupanicies[both]) & ((side == white) ? ~occupanicies[white] : ~occupanicies[black]);
                    while (attacks) {
                        target_square = GET_INDEX_OF_LSB1(attacks);
                        POP_BIT(attacks, target_square);
                        if (side == white && GET_BIT(occupanicies[black], target_square)){
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        else if(side == black && GET_BIT(occupanicies[white], target_square)){
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        else{
                        add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0));
                        }
                    }
                    break;
                case R:
                case r:
                    attacks = get_rook_attacks(source_square, occupanicies[both]) & ((side == white) ? ~occupanicies[white] : ~occupanicies[black]);
                    while (attacks) {
                        target_square = GET_INDEX_OF_LSB1(attacks);
                        POP_BIT(attacks, target_square);
                        if (side == white && GET_BIT(occupanicies[black], target_square)){
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        else if(side == black && GET_BIT(occupanicies[white], target_square)){
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        else{
                        add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0));
                        }
                    }
                    break;
                case Q:
                case q:
                    attacks = get_queen_attacks(source_square, occupanicies[both]) & ((side == white) ? ~occupanicies[white] : ~occupanicies[black]);
                    while (attacks) {
                        target_square = GET_INDEX_OF_LSB1(attacks);
                        POP_BIT(attacks, target_square);
                        if (side == white && GET_BIT(occupanicies[black], target_square)){
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        else if(side == black && GET_BIT(occupanicies[white], target_square)){
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        else{
                        add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0));
                        }
                    }
                    break;
                case K:
                case k:
                    attacks = king_attacks[source_square] & ((side == white) ? ~occupanicies[white] : ~occupanicies[black]);
                    while (attacks) {
                        target_square = GET_INDEX_OF_LSB1(attacks);
                        POP_BIT(attacks, target_square);
                        if (side == white && GET_BIT(occupanicies[black], target_square)){
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        else if(side == black && GET_BIT(occupanicies[white], target_square)){
                            add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        else{
                        add_move(move_list, ENCODE_MOVE(source_square, target_square, piece, 0, 0, 0, 0, 0));
                        }
                    }
                    break;
            }
        }
    }

}

//Make move function
enum {all_moves, captures};

const int castling_rights[64] = {
    7, 15, 15, 15, 3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 
    15, 15, 15, 15, 15, 15,15, 15, 
    13, 15, 15, 15, 12, 15,15, 14
};

static inline int make_move(int move, int move_flag){
    //non-capture moves
    if (move_flag == all_moves){
        copy_board();
        int source = GET_MOVE_SOURCE(move);
        int target = GET_MOVE_TARGET(move);
        int piece = GET_MOVE_PIECE(move);
        int promotion = GET_MOVE_PROMOTED(move);
        int capture = GET_MOVE_CAPTURE_FLAG(move);
        int double_push_move = GET_MOVE_DOUBLE_PUSH_FLAG(move);
        int enpassant_move = GET_MOVE_ENPASSANT_FLAG(move);
        int castling = GET_MOVE_CASTLING_FLAG(move);

        POP_BIT(bitboards[piece], source);
        SET_BIT(bitboards[piece], target);

        if(capture){
            int starting, ending;
            if(side == white){
                starting = p;
                ending = k;
            }
            else{
                starting = P;
                ending = K;
            }

            for (int bb = starting; bb <= ending; bb++){
                if(GET_BIT(bitboards[bb], target)){
                    POP_BIT(bitboards[bb],target );
                    break;
                }
            }
        }
        if(promotion){
            POP_BIT(bitboards[(side == white) ? P : p], target);
            SET_BIT(bitboards[promotion], target);
        }
        if(enpassant_move){
            (side == white) ? POP_BIT(bitboards[p], (target + 8)) : POP_BIT(bitboards[P], (target-8));
        }
        
        enpassant = no_square;
        
        if(double_push_move){
            (side == white) ? (enpassant = (target+8)) : (enpassant = (target-8));
        }

        if (castling){
            switch(target){
                case(G1):
                    POP_BIT(bitboards[R], H1);
                    SET_BIT(bitboards[R], F1);
                    break;
                
                case(C1):
                    POP_BIT(bitboards[R], A1);
                    SET_BIT(bitboards[R], D1);
                    break;
                
                case(G8):
                    POP_BIT(bitboards[r], H8);
                    SET_BIT(bitboards[r], F8);
                    break;
                
                case(C8):
                    POP_BIT(bitboards[r], A8);
                    SET_BIT(bitboards[r], D8);
                    break;
            }
        }

        castle &= castling_rights[source];
        castle &= castling_rights[target];

        memset(occupanicies, 0ULL, 24);
        for(int piece = P; piece <= K; piece++){
            occupanicies[white] |= bitboards[piece];
        }
        for (int bb_piece = p; bb_piece <= k; bb_piece++){
            occupanicies[black] |= bitboards[bb_piece];
        }
        occupanicies[both] |= occupanicies[white];
        occupanicies[both] |= occupanicies[black];

        // Check if the king is in check after the move
        side^=1;
        // make sure that king has not been exposed into a check
        if (square_attacked((side == white) ? GET_INDEX_OF_LSB1(bitboards[k]) : GET_INDEX_OF_LSB1(bitboards[K]), side))
        {
            // take move back
            restore_board();
            
            // return illegal move
            return 0;
        }
        else{
            // return legal move
            return 1;
        }

    }
    //capture moves
    else{
        if (GET_MOVE_CAPTURE_FLAG(move)){
            make_move(move,all_moves);
        }
        else{
            return 0;
        }
    }
    return 0;
}


long nodes;

static inline void perft(int depth){
    if (depth ==0){
        nodes++;
        return;
    }

    moves move_list[1];
    generate_moves(move_list);

    for (int move_count = 0; move_count < move_list->count; move_count++){
        copy_board();

        if(!make_move(move_list->moves_array[move_count], all_moves)){
           continue;
        }

        perft(depth-1);
        restore_board();
    }
}

void perf_test(int depth){
    std::cout<< "Performance Test: "<<std::endl;
    moves move_list[1];
    generate_moves(move_list);
    long start_time = time_check();

    for (int move_count = 0; move_count < move_list->count; move_count++){
        copy_board();

        if(!make_move(move_list->moves_array[move_count], all_moves)){
           continue;
        }

        long cum_nodes = nodes;
        perft(depth-1);
        long prev_nodes = nodes - cum_nodes;
        restore_board();
        printf("    move: %s%s%c nodes: %ld\n", square_to_coords[GET_MOVE_SOURCE(move_list->moves_array[move_count])],
                    square_to_coords[GET_MOVE_TARGET(move_list->moves_array[move_count])], 
                    promoted_pieces[GET_MOVE_PROMOTED(move_list->moves_array[move_count])], prev_nodes);
    }

    printf("Depth: %d\n", depth);
    printf("Nodes: %ld\n", nodes);
    printf("Time: %ld\n",(time_check() - start_time));
}

/*
--------------------
Evaluate section
--------------------
*/

int material[12] = {
    100, 300, 325, 500, 1000, 10000, // white pieces: P, N, B, R, Q, K
    -100, -300, -325, -500, -1000, -10000 // black pieces: p, n, b, r, q, k
};
// pawn positional score
const int pawn_score[64] = 
{
    90,  90,  90,  90,  90,  90,  90,  90,
    30,  30,  30,  40,  40,  30,  30,  30,
    20,  20,  20,  30,  30,  30,  20,  20,
    10,  10,  10,  20,  20,  10,  10,  10,
     5,   5,  10,  20,  20,   5,   5,   5,
     0,   0,   0,   5,   5,   0,   0,   0,
     0,   0,   0, -10, -10,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

// knight positional score
const int knight_score[64] = 
{
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,  10,  10,   0,   0,  -5,
    -5,   5,  20,  20,  20,  20,   5,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,   5,  20,  10,  10,  20,   5,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5, -10,   0,   0,   0,   0, -10,  -5
};

// bishop positional score
const int bishop_score[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,  10,  10,   0,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,  10,   0,   0,   0,   0,  10,   0,
     0,  30,   0,   0,   0,   0,  30,   0,
     0,   0, -10,   0,   0, -10,   0,   0

};

// rook positional score
const int rook_score[64] =
{
    50,  50,  50,  50,  50,  50,  50,  50,
    50,  50,  50,  50,  50,  50,  50,  50,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,   0,  20,  20,   0,   0,   0

};

// king positional score
const int king_score[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   5,   5,   5,   5,   0,   0,
     0,   5,   5,  10,  10,   5,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   0,   5,  10,  10,   5,   0,   0,
     0,   5,   5,  -5,  -5,   0,   5,   0,
     0,   0,   5,   0, -15,   0,  10,   0
};

// mirror positional score tables for opposite side
const int mirror_score[128] =
{
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8
};

int evaluate_pos() {
    int score = 0;
    uint64_t bitboard;

    // Loop through each piece type
    for (int piece = P; piece <= k; piece++) {
        bitboard = bitboards[piece];
        
        // Count the pieces on the board and update the score
        while (bitboard) {
            int square = GET_INDEX_OF_LSB1(bitboard);
            score += material[piece];

            // Add positional score
            switch (piece) {
                case P: score += pawn_score[square]; break;
                case N: score += knight_score[square]; break;
                case B: score += bishop_score[square]; break;
                case R: score += rook_score[square]; break;
                case K: score += king_score[square]; break;
                case p: score -= pawn_score[mirror_score[square]]; break;
                case n: score -= knight_score[mirror_score[square]]; break;
                case b: score -= bishop_score[mirror_score[square]]; break;
                case r: score -= rook_score[mirror_score[square]]; break;
                case k: score -= king_score[mirror_score[square]]; break;
            }

            POP_BIT(bitboard, square);
        }
    }

    // Return the final score
    return (side == white) ? score : -score;
}

/*
--------------------
Search section
--------------------
*/



//half move counter
int ply;

//stores best move
int best_move;

static inline int negamax(int alpha, int beta, int depth){
    if (depth == 0) return evaluate_pos();
    nodes++;
    moves move_list[1];
    generate_moves(move_list);

    //int max_eval = -50000;
    int old_a = alpha;
    int best_sofar = 0;

    for (int i = 0; i < move_list->count;i++){
        copy_board();
        ply++;
        if (make_move(move_list->moves_array[i], all_moves) == 0){
            ply--;
            continue;
        }

        int eval = -negamax(-beta, -alpha, depth-1);
        --ply;
        restore_board();

        if (eval >= beta) return beta;
        if (eval>alpha){
            alpha = eval;
            if (ply ==0){
                best_sofar = move_list->moves_array[i];
            }
        }
    }

    if (alpha != old_a){
        best_move = best_sofar;
    }

    return alpha;
}

void search_position(int depth)
{
    // find best move within a given position
    int score = negamax(-50000, 50000, depth);
    
    // best move placeholder
    printf("bestmove ");
    print_move(best_move);
    printf("\n");
}

/*
--------------------
Connecting to a GUI
--------------------
*/
//parse user/GUI move string input (e.g. "e7e8q") UNFINISHED

int parse_move(const char* move_string) {
    moves move_list[1];
    generate_moves(move_list);

    int source = (move_string[0] - 'a') + (8 - (move_string[1] - '0')) * 8;
    int target = (move_string[2] - 'a') + (8 - (move_string[3] - '0')) * 8;
    int promoted = 0;

    if (strlen(move_string) == 5) {
        switch (move_string[4]) {
            case 'q': promoted = (side == white) ? Q : q; break;
            case 'r': promoted = (side == white) ? R : r; break;
            case 'b': promoted = (side == white) ? B : b; break;
            case 'n': promoted = (side == white) ? N : n; break;
        }
    }

    for (int move_count = 0; move_count < move_list->count; move_count++) {
        int move = move_list->moves_array[move_count];
        if (GET_MOVE_SOURCE(move) == source &&
            GET_MOVE_TARGET(move) == target &&
            GET_MOVE_PROMOTED(move) == promoted) {
            return move;
        }
    }

    return 0;
}


void parse_position(const std::string& input_command){
    std::istringstream iss(input_command);
    std::string token;
    iss >> token;
    std::string fen;

    if (iss>>token){
        if (token == "startpos"){
            fen = start_position;
            if (iss >> token && token != "moves"){
                std::cerr << "Unexpected token after startpos: " <<token <<std::endl;
                return;
            }
        }
        else if(token == "fen"){
            while (iss >> token && token != "moves"){
                if (!fen.empty()){
                    fen +=" ";
                }
                fen+=token;
            }
        }
        else{
            std::cerr <<"Unknown position type: " <<token <<std::endl;
            return;
        }

        FEN_parse(fen.c_str());

        if (token == "moves"){
            while (iss>>token){
                int move = parse_move(token.c_str());
                if (move){
                    if(!make_move(move, all_moves)){
                        std::cerr << "Illegal move: " << token << std::endl;
                        return;
                    }
                }
                else{
                    std::cerr << "Failed to parse move: " <<token<<std::endl;
                }
            }
        }
    }
    else{
        std::cerr << "No position type specified" <<token<<std::endl;
        return;
    }
    print_board();
}

void parse_go(const std::string& input_command){
    std::istringstream iss(input_command);
    std::string token;
    int depth = -1;
    while(iss >> token){
        if (token == "depth" && iss >> token){
            depth = std::stoi(token);
        }
        else{
            depth =6;
        }
    }
    search_position(depth);
}

void uci_loop(){
    std::string line;

    std::cout.setf (std::ios::unitbuf);
    printf("id name ImplodsBeta\n");
    printf("id name Yash Dubbaka\n");
    printf("uciok\n");

    while (std::getline(std::cin, line)){
        if (line == "uci"){
			std::cout << "id name ImplodsBeta" << std::endl;
			std::cout << "id author Yash Dubbaka" << std::endl;
			std::cout << "uciok" << std::endl;
        }
        else if (line == "quit"){
            std::cout<<"Bye bye"<<std::endl;
            break;
        }
        else if ( line == "isready" ) {
			std::cout << "readyok" << std::endl;
        }
        else if (line == "ucinewgame"){
            parse_position("position startpos");
        }
        else if(line.substr(0,8) == "position"){
            parse_position(line);
        }
        else if(line.substr(0,2) == "go"){
            parse_go(line);
        }
    }
}



int main()
{

    init_all();
    int debug =0;
    if (debug){
        FEN_parse(start_position);
        print_board();
        search_position(6);
    }
    else{
    uci_loop();
    }
    return 0;
}

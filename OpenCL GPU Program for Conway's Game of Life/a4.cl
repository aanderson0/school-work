__kernel void breed(__global int board1[26][26], __global int board2[26][26]) {

    int rank;
    int min;
    int max;
    int numKernels;

    rank = get_global_id(0);
    numKernels = get_global_size(0);
    min = (rank * (26 / numKernels));
    if(rank == 0) min = 1;
    max = (rank + 1) * (26 / numKernels);
    if(rank == numKernels - 1) max = 25;
    
    for(int i = min; i < max; i++) {
        for(int j = 1; j < 26 - 1; j++){
            int neighbours = 0;
            if(board1[i-1][j+0]) neighbours++;
            if(board1[i-1][j-1]) neighbours++;
            if(board1[i+0][j-1]) neighbours++;
            if(board1[i+1][j+0]) neighbours++;
            if(board1[i+1][j+1]) neighbours++;
            if(board1[i+0][j+1]) neighbours++;
            if(board1[i-1][j+1]) neighbours++;
            if(board1[i+1][j-1]) neighbours++;
           
            if(board1[i][j] && neighbours == 2 || neighbours == 3) {
                board2[i][j] = 1;
            }
            else if(!board1[i][j] && neighbours == 3) {
                board2[i][j] = 1;
            }
            else {
                board2[i][j] = 0;
            }
        }
    }
}

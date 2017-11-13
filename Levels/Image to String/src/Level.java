/**
 * Holds and manipulates data for one flow level
 * @author Jonathan Bush
 */

import java.util.Arrays;
import java.util.ArrayList;
import java.io.File;
import java.io.IOException;
import javax.imageio.*;
import java.awt.image.BufferedImage;
public class Level {

    private int size;
    private ArrayList<Integer[]> nodes;

    
    public Level(int size){
        this();
        this.size = size;
    }
    
    public Level(){
        nodes = new ArrayList<Integer[]>();
    }
    
    public void rotate90(){
        for(Integer[] pair : nodes){
            for(int i = 0; i < 2; i++){
            int node = pair[i];
            int r = node/size;
            int c = node % size;
            Integer temp = c;
            c = r;
            r = size-1-temp;
            pair[i] = c+r*size;
            }
        }
    }
    
         
    public void setFromImg(BufferedImage img){
        this.size = 1;
        boolean same = false;
        for (int x = 14; x < 1184; x++){
            int rgb = img.getRGB(x, 408);
            int r = (rgb)&0xFF;
            int g = (rgb>>8)&0xFF;
            int b = (rgb>>16)&0xFF;
            if(r>55 || g>55 || b>55){
                if(!same){
                    ++size;
                    same = true;
                }
            } else {
                same = false;
            }
        }
        System.out.println("Size: " + size);
        
        int distance = 1200/size;
        int xOffset = distance/2;
        int yOffset = 400 + distance/2;
        
        int[][] board = new int[size][size];
        int node = 0;
        for (int r0 = 0; r0 < size; r0++){
            for (int c0 = 0; c0 < size; c0++){
                int rgb = img.getRGB(xOffset + distance*c0, yOffset + distance*r0);
                int r = (rgb)&0xFF;
                int g = (rgb>>8)&0xFF;
                int b = (rgb>>16)&0xFF;
                if(0 == board[r0][c0] && (r>50 || g>50 || b>50)){
                    nodes.add(new Integer[2]);
                    node++;
                    board[r0][c0] = node;
                    Integer[] temp = nodes.get(node-1);
                    temp[0] = c0 + size*r0;
                    for (int r1 = 0; r1 < size; r1++){
                        for (int c1 = 0; c1 < size; c1++){
                            if (!(r0 == r1 && c0 == c1) && rgb == img.getRGB(xOffset + distance*c1, yOffset + distance*r1)){
                                board[r1][c1] = node;
                                temp[1] = c1 + size*r1;
                            }
                        }
                    }
                }
            }
        }  
    }  

    
    public ArrayList<Integer[]> getNodes(){
        return nodes;
    }
    
    public int getDim(){
        return size;
    }
    
    public void randomizeNodeOrder(){
        for(int i = 0; i < nodes.size(); i++)
            nodes.add(i, nodes.remove((int)(Math.random()*nodes.size())));
        //System.out.println("randomized");
    }
}
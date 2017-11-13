import java.io.File;
import java.io.IOException;
import javax.imageio.*;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Scanner;

public class PackConverter {
 private LinkedList<Level> levels;
    

    public static void main(String[] args){
        PackConverter converter = new PackConverter();
        System.out.print("Directory path: ");
        Scanner input = new Scanner(System.in);
        converter.addDirectory(new File(input.next()));
        converter.randRotate();
        //converter.flipShuffle();
        converter.randShuffle();
        //Compression comp = new FancyCompress();
        //Compression comp = new FancierCompress();
        //Compression comp = new AwesomeCompress();
        //Compression comp = new PT_Compress();
        Compression comp = new FlowCEAppvarCompress();
        //System.out.print("\nStarting Level: ");
        //System.out.println(comp.packToString(converter.getPack(),input.nextInt()));
        System.out.println(comp.packToString(converter.getPack()));
    }
        
   
    
    public PackConverter(){
        levels = new LinkedList<>();
    }
    
    public void clearLevels(){
        levels = new LinkedList<>();
    }
    
    private void addDirectory(File dir){
        File[] directoryListing = dir.listFiles();
        if (directoryListing != null) {
            for (File child : directoryListing) {
                try{
                    BufferedImage img = ImageIO.read(child);

                    Level temp = new Level();
                    temp.setFromImg(img);
                    temp.randomizeNodeOrder();
                    levels.add(temp);

                } catch(IOException e){System.out.println("such error");}
                
            }
        } else {System.out.println("no files in directory");}
        System.out.println(levels.size());
    }
    
    public void flipShuffle(){
        int len = levels.size()-1;
        for(int i = 0; i < len; i++){
            int rand = (int)(Math.random()*len);
            levels.add(rand, levels.remove(rand+1));
        }
    }
    
    public void randShuffle(){
        for(int i = 0; i < levels.size(); i++)
            levels.add((int)(Math.random()*levels.size()), levels.remove(i));
    }
    
    public void randRotate(){
        for(Level lvl : levels){
            int n = (int)(4*Math.random());
            System.out.println("Rotate times: " + n);
            for(int i = 0; i < n; i++){
                lvl.rotate90();
            }
        }
    }
    
    public LinkedList<Level> getPack(){
        return levels;
    }
    
}
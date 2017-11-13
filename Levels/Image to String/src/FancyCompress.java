import java.util.ArrayList;
import java.util.LinkedList;

public class FancyCompress implements Compression {
    
    private static final String[] str0 = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P",
            "Q","R","S","T","U","V","W","X","Y","Z","0","1","2","3","4","5","6","7","8","9","theta"," ",
            "-","*","/","pi","ZoomStat","[","]","|E","{","}","If ","Then","Else","For(","While ","Repeat ",
            "End","Pause ","Lbl ","Goto ","IS>(","DS<(","prgm","Return","Stop","sin(","cos(","tan(",
            "sin^-1(","cos^-1(","tan^-1(","^^-1","^^2","sqrt(",",",">Dec","^^3","=","!=",">",">=","<","<=",
            " and "," or "," xor ","not(","Normal","Sci","Eng","Float","Radian","Degree","Func","Param",
            "Polar","Seq","^^o","'","^^r","|L","augment(",">DMS",">Frac","Boxplot","^^T","round(","pxl-Test(",
            "rowSwap(","row+(","*row(","*row+(","max(","min(","R>Pr(","R>Ptheta(","P>Rx(","P>Ry(","median(",
            "randM(","mean(","solve(","seq(","fnInt(","nDeriv(","fMin(","fMax(","!","CubicReg ","QuartReg ",
            "IndpntAuto","IndpntAsk","DependAsk","DependAuto","Trace","ClrDraw","ClrHome","Fill(","Shade(",
            "Circle(","Line(","ZStandard","ZTrig","ZBox","ZInteger","ZPrevious"};
    private int start;
    
    
    public String packToString(LinkedList<Level> pack){
        return packToString(pack, 0);
    }
    
    public String packToString(LinkedList<Level> pack, int start){
        this.start = start;
        
        String pckStr = "";
        
        for(int i = 0; i < pack.size(); i++){
            Level lvl = pack.get(i);
            pckStr += "\\+\\" +str0[start+i] + "\\" + str0[lvl.getDim()-1];
            ArrayList<Integer[]> nodes = lvl.getNodes();
            for(Integer[] pair : nodes){
                for(int node : pair){
                    //System.out.println(node);
                    pckStr += "\\"+ str0[node];
                }
            }
            System.out.println("Level " + (start+i) + " compressed");
        }
        
        return pckStr;
    }         
}  
import java.util.ArrayList;
import java.util.LinkedList;

public class FancierCompress implements Compression {
    
    private static final String[] str0 = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P",
        "Q","R","S","T","U","V","W","X","Y","Z","0","1","2","3","4","5","6","7","8","9","theta"," ","-",
        "*","/","pi","ZoomStat","[","]","|E","{","}","If ","Then","Else","For(","While ","Repeat ","End",
        "Pause ","Lbl ","Goto ","IS>(","DS<(","prgm","Return","Stop","sin(","cos(","tan(","sin^-1(",
        "cos^-1(","tan^-1(","^^-1","^^2","sqrt(",",",">Dec","^^3","=","!=",">",">=","<","<="," and ",
        " or "," xor ","not(","Normal","Sci","Eng","Float","Radian","Degree","Func","Param","Polar","Seq",
        "^^o","'","^^r","|L","augment(",">DMS",">Frac","Boxplot","^^T","round(","pxl-Test(","rowSwap(",
        "row+(","*row(","*row+(","max(","min(","R>Pr(","R>Ptheta(","P>Rx(","P>Ry(","median(","randM(",
        "mean(","solve(","seq(","fnInt(","nDeriv(","fMin(","fMax(","!","CubicReg ","QuartReg ","IndpntAuto",
        "IndpntAsk","DependAsk","DependAuto","Trace","ClrDraw","ClrHome","Fill(","Shade(","Circle(","Line(",
        "ZStandard","ZTrig","ZBox","ZInteger","ZPrevious","~","ZDecimal","ZoomRcl","Plot1(","ZoomSto",
        "Text("," nPr "," nCr ","FnOn ","LinReg(a+bx) ","StorePic ","StoreGDB ","RecallPic ","RecallGDB ",
        "ExpReg ","Vertical ","Pt-On(","Pt-Off(","Pt-Change(","Pxl-On(","Pxl-Off(","Pxl-Change(","ClrList ",
        "ClrTable","Horizontal ","Tangent(","DrawInv ","DrawF ","rand","getKey","?","int(","abs(","det(",
        "identity(","dim(","sum(","prod(","iPart(","fPart(","Histogram","cuberoot(","ln(","e^(","log(",
        "10^(","sinh(","sinh^-1(","cosh(","cosh^-1(","tanh(","tanh^-1("};
    
    
    public String packToString(LinkedList<Level> pack){
        
        String nodePosStr = "";
        String nodeNumStr = "";
        int i = 0;
        for (Level lvl : pack) {
            ++i;
            ArrayList<Integer[]> nodes = lvl.getNodes();
            nodeNumStr += "\\"+str0[nodes.size()*2];
            nodePosStr += "\\"+str0[lvl.getDim()-1];
            for(Integer[] pair : nodes){
                for(int node : pair){
                    nodePosStr += "\\"+ str0[node];
                }
            }
            System.out.println("Level " + (i) + " compressed");
        }
        
        return nodeNumStr + "\n" + nodePosStr;
    }         
}  
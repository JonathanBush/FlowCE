import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.LinkedList;

public class FlowCEAppvarCompress implements Compression {

    public String packToString(LinkedList<Level> pack){
        byte[] appVar;
        int numLevels = pack.size();
        String variableName = "FLPACK14";
        String packName = "14x14 Pack";
        String outputFilePath = "14x14.8xv";

        String packAppVarIdentifier = "FLCE";

        int totalNodes = 0;
        for (Level lvl : pack) {
            totalNodes += 2 * lvl.getNodes().size();
        }

        int packDataSize = 1 + 2*numLevels + totalNodes + 2 + packAppVarIdentifier.length() + packName.length() + 1;
            //packDataSize = 3;
        int dataSectionSize = 2 + 2 + 1 + 8 + 1 + 1 + 2 + packDataSize;
        int appVarSize = 8 + 3 + 42 + 2 + dataSectionSize + 2;

        appVar = new byte[appVarSize];

        byte[] signature = "**TI83F*".getBytes(StandardCharsets.US_ASCII);
        byte[] fSignature = {0x1A, 0x0A, 0x00};
        byte[] comment = new byte[42];
        String commentText = "FlowCE level pack data.";
        byte[] commentTextBytes = commentText.getBytes(StandardCharsets.US_ASCII);
        System.arraycopy(commentTextBytes, 0, comment, 0, commentTextBytes.length);
        byte[] dataLength = {(byte)(0xFF & (dataSectionSize)), (byte)(0xFF & ((dataSectionSize)>>8))};

        // construct that data section

        byte[] dataSection = new byte[dataSectionSize];
        //0x0D, 0x00
        byte[] variableHeader = {(byte)(0xFF & 0x0D), (byte)(0xFF & 0x00)};
        System.out.println(variableHeader[0]);
        System.arraycopy(variableHeader, 0, dataSection, 0, 2);
        System.out.println(dataSection[0]);
        byte[] variableDataLength = {(byte)(0xFF & (packDataSize)), (byte)(0xFF & (packDataSize) >> 8)};
        System.arraycopy(variableDataLength, 0, dataSection, 2, 2);
        dataSection[4] = 0x15;  // variable type ID
        byte[] variableNameBytes = variableName.getBytes(StandardCharsets.US_ASCII);
        System.arraycopy(variableNameBytes,0, dataSection, 5, variableNameBytes.length);
        dataSection[13] = 0x00; // version
        dataSection[14] = (byte)0x80; // archive flag 0x80 = archived, 0x00 = unarchived
        System.arraycopy(variableDataLength, 0, dataSection, 15, 2);
        System.out.println("packDataSize = " + packDataSize);
        byte[] variableDataLengthMinus2 = {(byte)(0xFF & (packDataSize-2)), (byte)(0xFF & (packDataSize-2) >> 8)};
        System.arraycopy(variableDataLengthMinus2, 0, dataSection, 17, 2);

        int i = 19;
        System.arraycopy(packAppVarIdentifier.getBytes(StandardCharsets.US_ASCII), 0, dataSection, i, packAppVarIdentifier.length());
        i += packAppVarIdentifier.length();
        dataSection[i++] = (byte)(0xFF & packName.length());
        System.arraycopy(packName.getBytes(StandardCharsets.US_ASCII), 0, dataSection, i, packName.length());
        i += packName.length();
        int j = i + 2*numLevels;
        //dataSection[i] = (byte)0xFF;
        //System.out.println(Arrays.toString(dataSection));
        dataSection[i] = (byte)(0xFF & numLevels);
        for (Level lvl : pack) {
            ArrayList<Integer[]> nodes = lvl.getNodes();
            dataSection[++i] = (byte)(0xFF & 2*nodes.size());   // length of data for this level
            dataSection[++i] = (byte)(0xFF & lvl.getDim());
            //dataSection[++j] = (byte)(0xFF & lvl.getDim());         // dimension of this level

            for(Integer[] pair : nodes){
                for(int node : pair){
                    dataSection[++j] = (byte)(0xFF & node);     // node position
                }
            }
        }

        byte[] checksum = computeDataChecksum(dataSection);

        System.arraycopy(signature,  0, appVar, 0, 8);
        System.arraycopy(fSignature, 0, appVar, 8, 3);
        System.arraycopy(comment,    0, appVar, 11, 42);
        System.arraycopy(dataLength, 0, appVar, 53, 2);
        System.arraycopy(dataSection,0, appVar, 55, dataSectionSize);
        System.arraycopy(checksum,   0, appVar, 55 + dataSectionSize, 2);
        System.out.println(appVar[55]);
        System.out.println(dataSectionSize);
        //write appVar to file
        try {
            FileOutputStream appVarStream = new FileOutputStream(outputFilePath);
            appVarStream.write(appVar);
        } catch (IOException e) {
            e.printStackTrace();
        }

        return "written to file";

    }

    private byte[] computeDataChecksum(byte[] data) {
        long sumOfBytes = 0;
        for (byte b : data) {
            sumOfBytes += 0xFF & (long)b;
        }
        return new byte[]{(byte)(0xFF & sumOfBytes), (byte)(0xFF & (sumOfBytes>>8))};
    }
}


public class Main {
    void main(String[] args) {
        if (args.length == 2) {
            var sequential = new Sequential();
            sequential.execute(args);
        } else if(args.length == 4){
            var parallel = new Parallel();
            parallel.execute(args);
        }
        else{
            System.err.println("Parameters: <data_folder> <output_file> for the sequential version");
            System.err.println("Parameters: <data_folder> <output_file> <number_of_threads> <reader_threads> for the parallel version");
            System.exit(1);
        }
    }
}

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <gtk/gtk.h>
#include <math.h>

typedef struct
{
    gint border_width;
} MyVals;

static MyVals bvals = {0};

////////////////////////////////////////////////////////////////////////////////

static gint xCoef(gint position, guchar* buf, gint width, gfloat coef, gint chanels){
  int multiplierMatrix[3][3] = {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
  gint a = multiplierMatrix[0][0] * buf[position - width - chanels];
  gint b = multiplierMatrix[0][1] * buf[position - width];
  gint c = multiplierMatrix[0][2] * buf[position - width + chanels];
  gint d = multiplierMatrix[1][0] * buf[position - chanels];
  gint e = multiplierMatrix[1][1] * buf[position];
  gint f = multiplierMatrix[1][2] * buf[position + chanels];
  gint g = multiplierMatrix[2][0] * buf[position + width - chanels];
  gint k = multiplierMatrix[2][1] * buf[position + width];
  gint l = multiplierMatrix[2][2] * buf[position + width + chanels];
  gint res = (a + b + c + d + e + f + g + k + l) * coef;
  return res;
}

static gint yCoef(gint position, guchar* buf, gint width, gfloat coef, gint chanels){
  int multiplierMatrix[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
  gint a = multiplierMatrix[0][0] * buf[position - width - chanels];
  gint b = multiplierMatrix[0][1] * buf[position - width];
  gint c = multiplierMatrix[0][2] * buf[position - width + chanels];
  gint d = multiplierMatrix[1][0] * buf[position - chanels];
  gint e = multiplierMatrix[1][1] * buf[position];
  gint f = multiplierMatrix[1][2] * buf[position + chanels];
  gint g = multiplierMatrix[2][0] * buf[position + width - chanels];
  gint k = multiplierMatrix[2][1] * buf[position + width];
  gint l = multiplierMatrix[2][2] * buf[position + width + chanels];
  gint res = (a + b + c + d + e + f + g + k + l) * coef;
  return res;
}

static void sobel(GimpDrawable *drawable){
    gint x1, y1, x2, y2, chanels; 
    gimp_drawable_mask_bounds(drawable->drawable_id, &x1, &y1, &x2, &y2); 
    GimpPixelRgn* region = (GimpPixelRgn *)g_new(GimpPixelRgn, 1);
    gimp_pixel_rgn_init(region, drawable, x1, y1, x2 - x1, y2 - y1, FALSE, FALSE);
    chanels = gimp_drawable_bpp(drawable->drawable_id);
    guchar* buf = (guchar*)g_new(guchar, (x2 - x1) * (y2 - y1) * chanels);
    gimp_pixel_rgn_get_rect(region, buf, x1, y1, x2 - x1, y2 - y1);
    guchar* bufOutput = (guchar*)g_new(guchar, (x2 - x1) * (y2 - y1) * chanels);
    
    gint width = (x2 - x1) * chanels; // in pixels
    gint height = y2 - y1;            // in pixels
    gint row = 2;                     // pixel strings
    //gfloat coef = (float)1/10;
    gfloat coef = 1;
    for (gint i = width + chanels; i < width * (height - 1); i++){
      
      if (i == row * width - chanels){
        i += 2 * chanels - 1;
        row++;
        continue;
      }
      gint x_value, y_value;
      x_value = xCoef(i, buf, width, coef, chanels);
      y_value = yCoef(i, buf, width, coef, chanels);
      x_value = (double)x_value;
      y_value = (double)y_value;
      bufOutput[i] = sqrt(pow(x_value, 2) + pow(y_value, 2));
    }
    gimp_pixel_rgn_set_rect(region, bufOutput, x1, y1, x2 - x1, y2 - y1);
    gimp_drawable_flush(drawable);
    gimp_drawable_update(drawable->drawable_id, x1, y1, x2 - x1, y2 - y1);
    free(buf);
    free(bufOutput);
    free(region);
}
//////////////////////////////////////////////////////////////////////////////////////

static gboolean border_dialog(GimpDrawable *drawable)
{
	GtkWidget *dialog;
	GtkWidget *main_vbox;
	GtkWidget *main_hbox;
	GtkWidget *frame;
	GtkWidget *radius_label;
	GtkWidget *alignment;
	GtkWidget *spinbutton;
	GtkObject *spinbutton_adj;
	GtkWidget *frame_label;
	gboolean   run;

	gimp_ui_init ("Frequency", FALSE);

	dialog = gimp_dialog_new ("My Frequency", "Frequency",
		                      NULL, 0,
		                      gimp_standard_help_func, "plug-in-Frequency",

		                      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		                      GTK_STOCK_OK,     GTK_RESPONSE_OK,

		                      NULL);

	main_vbox = gtk_vbox_new (FALSE, 6);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), main_vbox);
	gtk_widget_show (main_vbox);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (main_vbox), frame, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 6);

	alignment = gtk_alignment_new (0.5, 0.5, 1, 1);
	gtk_widget_show (alignment);
	gtk_container_add (GTK_CONTAINER (frame), alignment);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment), 6, 6, 6, 6);

	main_hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (main_hbox);
	gtk_container_add (GTK_CONTAINER (alignment), main_hbox);

	radius_label = gtk_label_new_with_mnemonic ("_Frequency:"); 
	gtk_widget_show (radius_label);
	gtk_box_pack_start (GTK_BOX (main_hbox), radius_label, FALSE, FALSE, 6);
	gtk_label_set_justify (GTK_LABEL (radius_label), GTK_JUSTIFY_RIGHT);

	spinbutton_adj = gtk_adjustment_new (3, 1, 500, 1, 5, 5);
	spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 0);
	gtk_widget_show (spinbutton);
	gtk_box_pack_start (GTK_BOX (main_hbox), spinbutton, FALSE, FALSE, 6);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinbutton), TRUE);

	frame_label = gtk_label_new ("<b>_Frequency</b>");
	gtk_widget_show (frame_label);
	gtk_frame_set_label_widget (GTK_FRAME (frame), frame_label);
	gtk_label_set_use_markup (GTK_LABEL (frame_label), TRUE);

	g_signal_connect (spinbutton_adj, "value_changed",
		              G_CALLBACK (gimp_int_adjustment_update),
		              &bvals.border_width);
	gtk_widget_show (dialog);

	run = (gimp_dialog_run (GIMP_DIALOG (dialog)) == GTK_RESPONSE_OK);

	gtk_widget_destroy (dialog);

	return run;
}


static void run (
   const gchar      *name,
   gint              nparams,
   const GimpParam  *param,
   gint             *nreturn_vals,
   GimpParam       **return_vals)
{
    static GimpParam  values[1];
    GimpPDBStatusType status = GIMP_PDB_SUCCESS;
    GimpRunMode       run_mode;
    GimpDrawable     *drawable;

    *nreturn_vals = 1;
    *return_vals  = values;

    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = status;

    run_mode = param[0].data.d_int32;
    gimp_progress_init("Process...");
    drawable = gimp_drawable_get(param[2].data.d_drawable);
    sobel(drawable);
}

static void query(void)
{
  static GimpParamDef args[] = {
    {
      GIMP_PDB_INT32,
      "run-mode",
      "Run mode"
    },
    {
      GIMP_PDB_IMAGE,
      "image",
      "Input image"
    },
    {
      GIMP_PDB_DRAWABLE,
      "drawable",
      "Input drawable"
    },
  };

  gimp_install_procedure (
    "plug-in-Frequency",
    "Frequency",
    "Makes image clearer.",
    "Novikov Timofey",
    "Copyright Novikov Timofey",
    "2022",
    "Frequency",
    "RGB*, GRAY*",
    GIMP_PLUGIN,
    G_N_ELEMENTS (args), 0,
    args, NULL);
    gimp_plugin_menu_register ("plug-in-Frequency",
                               "<Image>/Filters/Misc");
}

GimpPlugInInfo PLUG_IN_INFO = {
	NULL,
	NULL,
	query,
	run
};

MAIN()
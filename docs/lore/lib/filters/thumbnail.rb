class Thumbnailize < Nanoc::Filter

  identifier :thumbnailize
  type       :binary

  def run(filename, params={})
    system(
      'gm',
      'convert',
      '-sharpen', '1',
      '-thumbnail', params[:width].to_s+'x'+params[:height].to_s,
      '-gravity', 'center',
      '-background', 'none',
      '-extent', params[:width].to_s+'x'+params[:height].to_s,
      filename,
      "png:"+output_filename
    )
    system( 'optipng', '-quiet', output_filename )
  end

end

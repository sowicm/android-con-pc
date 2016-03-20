package com.sowicm.little;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.SurfaceHolder.Callback;
public class c360 extends SurfaceView implements Callback, Runnable
{
	private Thread th;
	private SurfaceHolder sfh;
	private Canvas canvas;
	private Paint paint;
	private boolean flag;
	private double rad;
	private double dis;
	//固定摇杆背景圆形的X,Y坐标以及半径
	private final float RockerCircleX = 300;
	private final float RockerCircleY = 300;
	private final float RockerCircleR = 200;
	private final float RockerCircleR2 = 40000;
	//摇杆的X,Y坐标以及摇杆的半径
	private float SmallRockerCircleX = 300;
	private float SmallRockerCircleY = 300;
	private final float SmallRockerCircleR = 60;
	public c360(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		sfh = getHolder();
		sfh.addCallback(this);
		paint = new Paint();
		paint.setAntiAlias(true);
		setFocusable(true);
		setFocusableInTouchMode(true);
	}
	public void surfaceCreated(SurfaceHolder holder) {
		th = new Thread(this);
		flag = true;
		th.start();
	}
	/***
	 * 得到两点之间的弧度
	 */
	public double getRad(float px1, float py1, float px2, float py2) {
		//得到两点X的距离
		double x = px2 - px1;
		//得到两点Y的距离
		double y = py1 - py2;
		//算出斜边长
		double xie = Math.sqrt(x * x + y * y);
		//得到这个角度的余弦值（通过三角函数中的定理 ：邻边/斜边=角度余弦值）
		double cosAngle = x / xie;
		//通过反余弦定理获取到其角度的弧度
		double rad = Math.acos(cosAngle);
		//注意：当触屏的位置Y坐标<摇杆的Y坐标我们要取反值-0~-180
		if (py2 < py1) {
			rad = -rad;
		}
		return rad;
	}
	@Override
	public boolean onTouchEvent(MotionEvent event)
	{
		int action = event.getAction();
		float f1, f2;
		if (action == MotionEvent.ACTION_DOWN || 
			action == MotionEvent.ACTION_MOVE)
		{
			// 当触屏区域不在活动范围内
			f1 = event.getX() - RockerCircleX;
			f2 = event.getY() - RockerCircleY;
			dis = f1 * f1 + f2 * f2;
			rad = getRad(RockerCircleX, RockerCircleY, event.getX(), event.getY());
			if (dis >= RockerCircleR2) {
			    dis = RockerCircleR2;
				//得到摇杆与触屏点所形成的角度
				//rad = getRad(RockerCircleX, RockerCircleY, event.getX(), event.getY());
				//保证内部小圆运动的长度限制
				getXY(RockerCircleX, RockerCircleY, RockerCircleR, rad);
			}
			else
			{
				//如果小球中心点小于活动区域则随着用户触屏点移动即可
				SmallRockerCircleX = event.getX();
				SmallRockerCircleY = event.getY();
			}
		}
		else if (event.getAction() == MotionEvent.ACTION_UP)
		{
			//当释放按键时摇杆要恢复摇杆的位置为初始位置
			SmallRockerCircleX = RockerCircleX;
			SmallRockerCircleY = RockerCircleY;
		}
		return true;
	}
	/**
	 * 
	 * @param R
	 *            圆周运动的旋转点
	 * @param centerX
	 *            旋转点X
	 * @param centerY
	 *            旋转点Y
	 * @param rad
	 *            旋转的弧度
	 */
	public void getXY(float centerX, float centerY, float R, double rad) {
		//获取圆周运动的X坐标 
		SmallRockerCircleX = (float) (R * Math.cos(rad)) + centerX;
		//获取圆周运动的Y坐标
		SmallRockerCircleY = (float) (R * Math.sin(rad)) + centerY;
	}
	public void draw()
	{
		try
		{
			canvas = sfh.lockCanvas();
			canvas.drawColor(Color.BLACK);
			//设置透明度
			paint.setColor(Color.WHITE);
			//绘制摇杆背景
			canvas.drawCircle(RockerCircleX, RockerCircleY, RockerCircleR, paint);
			paint.setColor(Color.BLACK);
			//绘制摇杆
			canvas.drawCircle(SmallRockerCircleX, SmallRockerCircleY, 
					SmallRockerCircleR, paint);
			
		    if (RockerCircleX != SmallRockerCircleX ||
		    	RockerCircleY != SmallRockerCircleY)
		    {
		    	String str = little.commands.mouseMove.toString();
		    	str += "#";
		    	double dt = rad * 100000.0;
		    	str += String.valueOf((int) dt);
		    	str += "#";
		    	dt = dis / RockerCircleR2;
		    	dt *= 100000.0;
		    	str += String.valueOf((int) dt);
		    	little.sendCmd(str);
		    }
		    else
		    {
		        little.sendCmd(little.commands.mouseMove.toString());
		    }
		}
		catch (Exception e)
		{
			// TODO: handle exception
		}
		finally
		{
			try
			{
				if (canvas != null)
					sfh.unlockCanvasAndPost(canvas);
			}
			catch (Exception e)
			{
			}
		}
    }
    public void run()
    {
        while (flag)
        {
            draw();
            try
            {
                Thread.sleep(200);// 50);
            }
            catch (Exception e)
            {
            }
        }
    }
    public void surfaceChanged(SurfaceHolder holder, int format, int width,
            int height)
    {

    }
    public void surfaceDestroyed(SurfaceHolder holder)
    {
        flag = false;
    }
}
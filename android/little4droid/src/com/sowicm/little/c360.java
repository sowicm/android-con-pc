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
	//�̶�ҡ�˱���Բ�ε�X,Y�����Լ��뾶
	private final float RockerCircleX = 300;
	private final float RockerCircleY = 300;
	private final float RockerCircleR = 200;
	private final float RockerCircleR2 = 40000;
	//ҡ�˵�X,Y�����Լ�ҡ�˵İ뾶
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
	 * �õ�����֮��Ļ���
	 */
	public double getRad(float px1, float py1, float px2, float py2) {
		//�õ�����X�ľ���
		double x = px2 - px1;
		//�õ�����Y�ľ���
		double y = py1 - py2;
		//���б�߳�
		double xie = Math.sqrt(x * x + y * y);
		//�õ�����Ƕȵ�����ֵ��ͨ�����Ǻ����еĶ��� ���ڱ�/б��=�Ƕ�����ֵ��
		double cosAngle = x / xie;
		//ͨ�������Ҷ����ȡ����ǶȵĻ���
		double rad = Math.acos(cosAngle);
		//ע�⣺��������λ��Y����<ҡ�˵�Y��������Ҫȡ��ֵ-0~-180
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
			// �����������ڻ��Χ��
			f1 = event.getX() - RockerCircleX;
			f2 = event.getY() - RockerCircleY;
			dis = f1 * f1 + f2 * f2;
			rad = getRad(RockerCircleX, RockerCircleY, event.getX(), event.getY());
			if (dis >= RockerCircleR2) {
			    dis = RockerCircleR2;
				//�õ�ҡ���봥�������γɵĽǶ�
				//rad = getRad(RockerCircleX, RockerCircleY, event.getX(), event.getY());
				//��֤�ڲ�СԲ�˶��ĳ�������
				getXY(RockerCircleX, RockerCircleY, RockerCircleR, rad);
			}
			else
			{
				//���С�����ĵ�С�ڻ�����������û��������ƶ�����
				SmallRockerCircleX = event.getX();
				SmallRockerCircleY = event.getY();
			}
		}
		else if (event.getAction() == MotionEvent.ACTION_UP)
		{
			//���ͷŰ���ʱҡ��Ҫ�ָ�ҡ�˵�λ��Ϊ��ʼλ��
			SmallRockerCircleX = RockerCircleX;
			SmallRockerCircleY = RockerCircleY;
		}
		return true;
	}
	/**
	 * 
	 * @param R
	 *            Բ���˶�����ת��
	 * @param centerX
	 *            ��ת��X
	 * @param centerY
	 *            ��ת��Y
	 * @param rad
	 *            ��ת�Ļ���
	 */
	public void getXY(float centerX, float centerY, float R, double rad) {
		//��ȡԲ���˶���X���� 
		SmallRockerCircleX = (float) (R * Math.cos(rad)) + centerX;
		//��ȡԲ���˶���Y����
		SmallRockerCircleY = (float) (R * Math.sin(rad)) + centerY;
	}
	public void draw()
	{
		try
		{
			canvas = sfh.lockCanvas();
			canvas.drawColor(Color.BLACK);
			//����͸����
			paint.setColor(Color.WHITE);
			//����ҡ�˱���
			canvas.drawCircle(RockerCircleX, RockerCircleY, RockerCircleR, paint);
			paint.setColor(Color.BLACK);
			//����ҡ��
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